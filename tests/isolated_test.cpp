#include <QTemporaryDir>

#include <gtest/gtest.h>
#include <holonight_thumbnails/isolated.h>
TEST(IsolatedCache, RuntimeFailureIsNotAMissAndCancellationIsSeparate) {
  QTemporaryDir home;
  ASSERT_TRUE(home.isValid());
  const auto prior = qgetenv("XDG_CACHE_HOME");
  qputenv("XDG_CACHE_HOME", home.path().toUtf8());
  const HolonightThumbnails::Request request{
      .uri = QUrl::fromLocalFile(home.filePath("source")),
      .modified = QDateTime::fromSecsSinceEpoch(100),
      .size = 10,
      .required = {8, 8},
  };
  std::atomic_bool cancelled{false};
  const HolonightImages::IsolatedClient client(home.filePath("missing-runtime"));
  auto miss = HolonightThumbnails::lookupIsolated(request, client, cancelled);
  EXPECT_EQ(miss.runtime, HolonightImages::RuntimeStatus::Success);
  EXPECT_FALSE(miss.image);
  QImage pixels(8, 8, QImage::Format_RGB32);
  pixels.fill(Qt::red);
  EXPECT_TRUE(HolonightThumbnails::store(request, pixels, cancelled));
  auto failure = HolonightThumbnails::lookupIsolated(request, client, cancelled);
  EXPECT_EQ(failure.runtime, HolonightImages::RuntimeStatus::Unavailable);
  EXPECT_FALSE(failure.image);
  EXPECT_FALSE(failure.diagnostic.isEmpty());
  cancelled.store(true);
  EXPECT_EQ(HolonightThumbnails::lookupIsolated(request, client, cancelled).runtime,
            HolonightImages::RuntimeStatus::Cancelled);
  if (prior.isNull()) {
    qunsetenv("XDG_CACHE_HOME");
  } else {
    qputenv("XDG_CACHE_HOME", prior);
  }
}
