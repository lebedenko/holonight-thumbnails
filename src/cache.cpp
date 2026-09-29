#include <holonight_thumbnails/cache.h>

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QStandardPaths>

#include <array>
#include <cstring>

namespace HolonightThumbnails {
namespace {

constexpr std::array kTiers = {128, 256, 512, 1024};
constexpr qint64 kMaximumPngBytes = 32 * 1024 * 1024;
constexpr auto kOrientationPolicy = "applied-v1";
constexpr auto kSvgPolicy = "self-contained-static-v1";

QString tierName(int tier) {
  switch (tier) {
    case 128: return QStringLiteral("normal");
    case 256: return QStringLiteral("large");
    case 512: return QStringLiteral("x-large");
    case 1024: return QStringLiteral("xx-large");
    default: return {};
  }
}

QString basePath() {
  return QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QStringLiteral("/thumbnails");
}

QString canonicalUri(const QUrl& uri) {
  return QUrl::fromLocalFile(QDir::cleanPath(uri.toLocalFile())).toString(QUrl::FullyEncoded);
}

bool sourceIsCacheEntry(const QUrl& uri) {
  const auto source = QDir::cleanPath(uri.toLocalFile());
  const auto root = QDir::cleanPath(basePath());
  return source == root || source.startsWith(root + QLatin1Char('/'));
}

bool validRequest(const Request& request) {
  return request.uri.isValid() && request.uri.isLocalFile() &&
         !request.uri.hasQuery() && !request.uri.hasFragment() &&
         QFileInfo(request.uri.toLocalFile()).isAbsolute() && !sourceIsCacheEntry(request.uri) &&
         request.modified.isValid() && request.size >= 0 && tierForSize(request.required) &&
         (request.tier == 0 || (!tierName(request.tier).isEmpty() &&
                                request.tier >= *tierForSize(request.required)));
}

int selectedTier(const Request& request) {
  return request.tier == 0 ? *tierForSize(request.required) : request.tier;
}

bool matches(const QImage& image, const Request& request) {
  if (image.isNull() || image.width() < request.required.width() || image.height() < request.required.height() ||
      image.text(QStringLiteral("Thumb::URI")) != canonicalUri(request.uri)) {
    return false;
  }
  bool okay = false;
  const auto seconds = image.text(QStringLiteral("Thumb::MTime")).toLongLong(&okay);
  if (!okay || seconds != request.modified.toSecsSinceEpoch()) {
    return false;
  }
  const auto sizeText = image.text(QStringLiteral("Thumb::Size"));
  if (!sizeText.isEmpty() && (sizeText.toLongLong(&okay) != request.size || !okay)) {
    return false;
  }
  const auto millisecondsText = image.text(QStringLiteral("X-HoloNight::MTimeMsec"));
  if (!millisecondsText.isEmpty() &&
      (millisecondsText.toLongLong(&okay) != request.modified.toMSecsSinceEpoch() || !okay)) {
    return false;
  }
  const auto orientation = image.text(QStringLiteral("Files::OrientationPolicy"));
  const auto sharedOrientation = image.text(QStringLiteral("X-HoloNight::OrientationPolicy"));
  if ((!orientation.isEmpty() && orientation != QLatin1String(kOrientationPolicy)) ||
      (!sharedOrientation.isEmpty() && sharedOrientation != QLatin1String(kOrientationPolicy))) {
    return false;
  }
  const auto legacySvg = image.text(QStringLiteral("Files::SvgPolicy"));
  const auto sharedSvg = image.text(QStringLiteral("X-HoloNight::SvgPolicy"));
  if (request.kind == Kind::Svg) {
    if ((legacySvg.isEmpty() && sharedSvg.isEmpty()) ||
        (!legacySvg.isEmpty() && legacySvg != QLatin1String(kSvgPolicy)) ||
        (!sharedSvg.isEmpty() && sharedSvg != QLatin1String(kSvgPolicy))) {
      return false;
    }
  } else if (!legacySvg.isEmpty() || !sharedSvg.isEmpty()) {
    return false;
  }
  const auto oldRevision = image.text(QStringLiteral("Files::Revision"));
  if (!request.revision.isEmpty() && !oldRevision.isEmpty() && oldRevision != request.revision) {
    return false;
  }
  return true;
}

bool isBoundedPng(const QByteArray& bytes, int tier) {
  constexpr char signature[] = "\x89PNG\r\n\x1a\n";
  if (bytes.size() < 33 || memcmp(bytes.constData(), signature, 8) != 0 ||
      memcmp(bytes.constData() + 12, "IHDR", 4) != 0) {
    return false;
  }
  const auto* data = reinterpret_cast<const unsigned char*>(bytes.constData());
  auto number = [&](int offset) -> quint32 {
    return (quint32(data[offset]) << 24) | (quint32(data[offset + 1]) << 16) |
           (quint32(data[offset + 2]) << 8) | quint32(data[offset + 3]);
  };
  const auto width = number(16);
  const auto height = number(20);
  return number(8) == 13 && width > 0 && height > 0 && width <= quint32(tier) && height <= quint32(tier);
}

bool ensureDirectory(const QString& path) {
  const auto base = basePath();
  if (!QDir().mkpath(path)) {
    return false;
  }
  const auto ownerOnly = QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner;
  return QFile::setPermissions(base, ownerOnly) && QFile::setPermissions(path, ownerOnly);
}

}  // namespace

std::optional<int> tierForSize(QSize required) {
  if (!required.isValid() || required.isEmpty()) {
    return std::nullopt;
  }
  for (const int tier : kTiers) {
    if (required.width() <= tier && required.height() <= tier) {
      return tier;
    }
  }
  return std::nullopt;
}

QString cachePath(const QUrl& uri, int tier) {
  if (!uri.isValid() || !uri.isLocalFile() || !QFileInfo(uri.toLocalFile()).isAbsolute() ||
      uri.hasQuery() || uri.hasFragment() || tierName(tier).isEmpty()) {
    return {};
  }
  const auto hash = QCryptographicHash::hash(canonicalUri(uri).toUtf8(), QCryptographicHash::Md5).toHex();
  return basePath() + QLatin1Char('/') + tierName(tier) + QLatin1Char('/') + QString::fromLatin1(hash) +
         QStringLiteral(".png");
}

std::optional<QImage> lookup(const Request& request, const std::atomic_bool& cancelled, const StageCallback& stage) {
  if (cancelled.load() || !validRequest(request)) {
    return std::nullopt;
  }
  for (const int tier : kTiers) {
    if (cancelled.load()) {
      return std::nullopt;
    }
    if (tier < selectedTier(request)) {
      continue;
    }
    QFile file(cachePath(request.uri, tier));
    if (!file.open(QIODevice::ReadOnly) || file.size() > kMaximumPngBytes) {
      continue;
    }
    if (stage) stage(Stage::CacheInspect);
    if (cancelled.load()) return std::nullopt;
    const auto bytes = file.read(kMaximumPngBytes + 1);
    if (stage) stage(Stage::CacheInspected);
    if (cancelled.load()) return std::nullopt;
    if (bytes.size() > kMaximumPngBytes || !isBoundedPng(bytes, tier)) {
      continue;
    }
    if (stage) stage(Stage::CacheDecode);
    if (cancelled.load()) return std::nullopt;
    QImage image;
    if (image.loadFromData(bytes, "PNG") && matches(image, request) && !cancelled.load()) {
      return image;
    }
  }
  return std::nullopt;
}

bool store(const Request& request, const QImage& image, const std::atomic_bool& cancelled, const StageCallback& stage) {
  if (cancelled.load() || !validRequest(request) || image.isNull()) {
    return false;
  }
  const int tier = selectedTier(request);
  if (image.width() > tier || image.height() > tier || image.width() < request.required.width() ||
      image.height() < request.required.height()) {
    return false;
  }
  const auto path = cachePath(request.uri, tier);
  if (!ensureDirectory(QFileInfo(path).absolutePath()) || cancelled.load()) {
    return false;
  }
  QImage tagged = image;
  tagged.setText(QStringLiteral("Thumb::URI"), canonicalUri(request.uri));
  tagged.setText(QStringLiteral("Thumb::MTime"), QString::number(request.modified.toSecsSinceEpoch()));
  tagged.setText(QStringLiteral("Thumb::Size"), QString::number(request.size));
  tagged.setText(QStringLiteral("X-HoloNight::MTimeMsec"), QString::number(request.modified.toMSecsSinceEpoch()));
  tagged.setText(QStringLiteral("X-HoloNight::OrientationPolicy"), QLatin1String(kOrientationPolicy));
  tagged.setText(QStringLiteral("Files::OrientationPolicy"), QLatin1String(kOrientationPolicy));
  tagged.setText(QStringLiteral("Files::Revision"), {});
  if (!request.revision.isEmpty()) tagged.setText(QStringLiteral("Files::Revision"), request.revision);
  if (request.kind == Kind::Svg) {
    tagged.setText(QStringLiteral("X-HoloNight::SvgPolicy"), QLatin1String(kSvgPolicy));
    tagged.setText(QStringLiteral("Files::SvgPolicy"), QLatin1String(kSvgPolicy));
  } else {
    tagged.setText(QStringLiteral("X-HoloNight::SvgPolicy"), {});
    tagged.setText(QStringLiteral("Files::SvgPolicy"), {});
  }
  QSaveFile file(path);
  if (!file.open(QIODevice::WriteOnly)) {
    return false;
  }
  if (!file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner)) {
    file.cancelWriting();
    return false;
  }
  if (cancelled.load() || !tagged.save(&file, "PNG")) {
    file.cancelWriting();
    return false;
  }
  if (stage) stage(Stage::BeforeCommit);
  if (cancelled.load()) {
    file.cancelWriting();
    return false;
  }
  if (!file.commit()) {
    return false;
  }
  QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
  return true;
}

}  // namespace HolonightThumbnails
