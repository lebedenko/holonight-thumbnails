#pragma once
#include <holonight_thumbnails/cache.h>
namespace HolonightThumbnails::detail {
using Decoder = std::function<std::optional<QImage>(const QByteArray&, int, bool&)>;
std::optional<QImage> lookup(const Request& request, const std::atomic_bool& cancelled, const StageCallback& stage,
                             const Decoder& decode);
}  // namespace HolonightThumbnails::detail
