#include "lookup.h"

#include <QBuffer>

#include <holonight_thumbnails/isolated.h>
namespace HolonightThumbnails {
IsolatedLookup lookupIsolated(const Request& request, const HolonightImages::IsolatedClient& client,
                              const std::atomic_bool& cancelled, const StageCallback& stage) {
  IsolatedLookup result;
  result.image = detail::lookup(
      request, cancelled, stage, [&](const QByteArray& bytes, int tier, bool& stop) -> std::optional<QImage> {
        QBuffer source;
        source.setData(bytes);
        source.open(QIODevice::ReadOnly);
        const HolonightImages::Limits limits{
            .inputBytes = 32 * 1024 * 1024,
            .sourcePixels = tier * tier,
            .sourceExtent = tier,
            .decodedBytes = 32 * 1024 * 1024,
            .metadataBytes = 1024 * 1024,
            .tiffMetadataBytes = 1024 * 1024,
            .metadataRecords = 4096,
        };
        auto decoded = client.decode(
            source,
            {.limits = limits, .bound = {tier, tier}, .orientation = HolonightImages::OrientationPolicy::Ignore},
            cancelled);
        result.runtime = decoded.runtime;
        result.diagnostic = decoded.diagnostic;
        stop = decoded.runtime != HolonightImages::RuntimeStatus::Success;
        if (stop || !decoded.result || decoded.result->outcome != HolonightImages::Outcome::Success ||
            decoded.result->inspection.format != "png") {
          return std::nullopt;
        }
        return std::move(decoded.result->image);
      });
  if (cancelled.load() && result.runtime == HolonightImages::RuntimeStatus::Success) {
    result.image.reset();
    result.runtime = HolonightImages::RuntimeStatus::Cancelled;
  }
  return result;
}
std::optional<QImage> lookupIsolated(const Request& request, HolonightImages::SourceTransaction& transaction,
                                     const std::atomic_bool& cancelled, const StageCallback& stage) {
  return detail::lookup(
      request, cancelled, stage, [&](const QByteArray& bytes, int tier, bool&) -> std::optional<QImage> {
        QBuffer source;
        source.setData(bytes);
        source.open(QIODevice::ReadOnly);
        const HolonightImages::Limits limits{
            .inputBytes = 32 * 1024 * 1024,
            .sourcePixels = tier * tier,
            .sourceExtent = tier,
            .decodedBytes = 32 * 1024 * 1024,
            .metadataBytes = 1024 * 1024,
            .tiffMetadataBytes = 1024 * 1024,
            .metadataRecords = 4096,
        };
        auto decoded = transaction.decodeCachedPng(
            source,
            {.limits = limits, .bound = {tier, tier}, .orientation = HolonightImages::OrientationPolicy::Ignore});
        if (decoded.outcome != HolonightImages::Outcome::Success || decoded.inspection.format != "png") {
          return std::nullopt;
        }
        return std::move(decoded.image);
      });
}
}  // namespace HolonightThumbnails
