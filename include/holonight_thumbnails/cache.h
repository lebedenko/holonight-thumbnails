#pragma once

#include <QDateTime>
#include <QImage>
#include <QSize>
#include <QString>
#include <QUrl>

#include <atomic>
#include <functional>
#include <optional>

namespace HolonightThumbnails {

enum class Kind { Raster, Svg };
enum class Stage { CacheInspect, CacheInspected, CacheDecode, BeforeCommit };
using StageCallback = std::function<void(Stage)>;

struct Request {
  QUrl uri;
  QDateTime modified;
  qint64 size = -1;
  QSize required;
  Kind kind = Kind::Raster;
  QString revision;
  int tier = 0;  // Zero selects the smallest tier fitting required.
};

// Returns no tier for invalid dimensions or requests larger than 1024 pixels.
std::optional<int> tierForSize(QSize required);
// Returns an empty path for an invalid URI or tier. Does not create directories.
QString cachePath(const QUrl& uri, int tier);
// A missing, stale, invalid, cancelled, or unreadable entry is a cache miss.
std::optional<QImage> lookup(const Request& request, const std::atomic_bool& cancelled,
                             const StageCallback& stage = {});
// A failed or cancelled write is harmless to the caller's decoded image.
bool store(const Request& request, const QImage& image, const std::atomic_bool& cancelled,
           const StageCallback& stage = {});

}  // namespace HolonightThumbnails
