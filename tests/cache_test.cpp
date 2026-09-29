#include <holonight_thumbnails/cache.h>

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

#include <gtest/gtest.h>

#include <thread>
#include <vector>

using namespace HolonightThumbnails;

namespace {

class CacheTest : public ::testing::Test {
 protected:
  QTemporaryDir home;
  QByteArray prior = qgetenv("XDG_CACHE_HOME");
  bool hadPrior = qEnvironmentVariableIsSet("XDG_CACHE_HOME");
  QString source;
  Request request;
  std::atomic_bool cancelled{false};

  void SetUp() override {
    ASSERT_TRUE(home.isValid());
    qputenv("XDG_CACHE_HOME", home.path().toUtf8());
    source = home.filePath("source.png");
    QFile file(source);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    ASSERT_EQ(file.write("source", 6), 6);
    file.close();
    request = {.uri = QUrl::fromLocalFile(source), .modified = QFileInfo(source).lastModified(),
               .size = QFileInfo(source).size(), .required = {100, 100}};
  }

  void TearDown() override {
    if (hadPrior) qputenv("XDG_CACHE_HOME", prior);
    else qunsetenv("XDG_CACHE_HOME");
  }

  QImage image(int width = 128, int height = 128) {
    QImage value(width, height, QImage::Format_ARGB32);
    value.fill(Qt::red);
    return value;
  }

  void external(const QImage& pixels, int tier = 128) {
    const auto path = cachePath(request.uri, tier);
    ASSERT_TRUE(QDir().mkpath(QFileInfo(path).absolutePath()));
    ASSERT_TRUE(pixels.save(path, "PNG"));
  }
};

TEST_F(CacheTest, SelectsFreedesktopTiersAndUriHash) {
  EXPECT_EQ(tierForSize({1, 1}), 128);
  EXPECT_EQ(tierForSize({129, 1}), 256);
  EXPECT_EQ(tierForSize({257, 1}), 512);
  EXPECT_EQ(tierForSize({513, 1}), 1024);
  EXPECT_FALSE(tierForSize({1025, 1}));
  const auto digest = QCryptographicHash::hash(request.uri.toString(QUrl::FullyEncoded).toUtf8(),
                                               QCryptographicHash::Md5).toHex();
  EXPECT_TRUE(cachePath(request.uri, 128).endsWith("/thumbnails/normal/" + QString::fromLatin1(digest) + ".png"));
  const auto redundant = QUrl::fromLocalFile(home.filePath("folder/../source.png"));
  EXPECT_EQ(cachePath(redundant, 128), cachePath(request.uri, 128));
}

TEST_F(CacheTest, StoresAndReadsWithPrivatePermissions) {
  ASSERT_TRUE(store(request, image(), cancelled));
  auto found = lookup(request, cancelled);
  ASSERT_TRUE(found);
  EXPECT_EQ(found->pixelColor(0, 0), QColor(Qt::red));
  EXPECT_EQ(found->text("Thumb::Size"), QString::number(request.size));
  EXPECT_EQ(found->text("Files::OrientationPolicy"), "applied-v1");
  EXPECT_EQ(QFileInfo(cachePath(request.uri, 128)).permissions() & QFileDevice::ReadGroup, 0);
  EXPECT_EQ(QFileInfo(cachePath(request.uri, 128)).permissions() & QFileDevice::ReadOther, 0);
}

TEST_F(CacheTest, ReusesExternalRasterWithStandardMetadata) {
  auto pixels = image();
  pixels.setText("Thumb::URI", request.uri.toString(QUrl::FullyEncoded));
  pixels.setText("Thumb::MTime", QString::number(request.modified.toSecsSinceEpoch()));
  external(pixels);
  EXPECT_TRUE(lookup(request, cancelled));
  request.modified = request.modified.addSecs(1);
  EXPECT_FALSE(lookup(request, cancelled));
}

TEST_F(CacheTest, RejectsCorruptUndersizedAndMismatchedEntries) {
  ASSERT_TRUE(store(request, image(), cancelled));
  request.required = {129, 129};
  EXPECT_FALSE(lookup(request, cancelled));
  request.required = {100, 100};
  request.size++;
  EXPECT_FALSE(lookup(request, cancelled));
  request.size--;
  QFile file(cachePath(request.uri, 128));
  ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
  ASSERT_EQ(file.write("bad", 3), 3);
  file.close();
  EXPECT_FALSE(lookup(request, cancelled));
}

TEST_F(CacheTest, RejectsOversizedPngBeforeDecode) {
  auto pixels = image(129, 100);
  pixels.setText("Thumb::URI", request.uri.toString(QUrl::FullyEncoded));
  pixels.setText("Thumb::MTime", QString::number(request.modified.toSecsSinceEpoch()));
  external(pixels);
  EXPECT_FALSE(lookup(request, cancelled));
}

TEST_F(CacheTest, ChecksStrongerHoloNightModificationTime) {
  ASSERT_TRUE(store(request, image(), cancelled));
  request.modified = request.modified.addMSecs(1);
  EXPECT_FALSE(lookup(request, cancelled));
}

TEST_F(CacheTest, RequiresSvgPolicyAndRejectsWrongLegacyPolicy) {
  request.kind = Kind::Svg;
  auto pixels = image();
  pixels.setText("Thumb::URI", request.uri.toString(QUrl::FullyEncoded));
  pixels.setText("Thumb::MTime", QString::number(request.modified.toSecsSinceEpoch()));
  external(pixels);
  EXPECT_FALSE(lookup(request, cancelled));
  pixels.setText("Files::SvgPolicy", "self-contained-static-v1");
  external(pixels);
  EXPECT_TRUE(lookup(request, cancelled));
  pixels.setText("Files::OrientationPolicy", "other");
  external(pixels);
  EXPECT_FALSE(lookup(request, cancelled));
}

TEST_F(CacheTest, CancellationAtCommitKeepsPreviousImage) {
  ASSERT_TRUE(store(request, image(), cancelled));
  auto blue = image();
  blue.fill(Qt::blue);
  EXPECT_FALSE(store(request, blue, cancelled, [&](Stage stage) {
    if (stage == Stage::BeforeCommit) cancelled.store(true);
  }));
  cancelled.store(false);
  auto found = lookup(request, cancelled);
  ASSERT_TRUE(found);
  EXPECT_EQ(found->pixelColor(0, 0), QColor(Qt::red));
}

TEST_F(CacheTest, ConcurrentStoresLeaveReadableEntry) {
  std::vector<std::thread> writers;
  for (int i = 0; i < 8; ++i) {
    writers.emplace_back([&, i] {
      auto pixels = image();
      pixels.fill(i % 2 ? Qt::red : Qt::blue);
      store(request, pixels, cancelled);
    });
  }
  for (auto& writer : writers) writer.join();
  auto found = lookup(request, cancelled);
  ASSERT_TRUE(found);
  EXPECT_TRUE(found->pixelColor(0, 0) == QColor(Qt::red) || found->pixelColor(0, 0) == QColor(Qt::blue));
}

TEST_F(CacheTest, WriteFailureIsMiss) {
  const auto thumbnails = home.filePath("thumbnails");
  QFile blocker(thumbnails);
  ASSERT_TRUE(blocker.open(QIODevice::WriteOnly));
  blocker.close();
  EXPECT_FALSE(store(request, image(), cancelled));
  EXPECT_FALSE(lookup(request, cancelled));
}

TEST_F(CacheTest, DoesNotThumbnailTheCacheItself) {
  request.uri = QUrl::fromLocalFile(home.filePath("thumbnails/normal/a.png"));
  EXPECT_FALSE(store(request, image(), cancelled));
}

}  // namespace
