#pragma once
#include <holonight_images/isolated.h>
#include <holonight_thumbnails/cache.h>
namespace HolonightThumbnails {
struct IsolatedLookup {
  std::optional<QImage> image;
  HolonightImages::RuntimeStatus runtime = HolonightImages::RuntimeStatus::Success;
  QString diagnostic;
};
// Cache misses are successful runtime operations without an image. Infrastructure
// failures terminate traversal. The caller owns the client and cancellation flag.
IsolatedLookup lookupIsolated(const Request& request, const HolonightImages::IsolatedClient& client,
                              const std::atomic_bool& cancelled, const StageCallback& stage = {});
}  // namespace HolonightThumbnails
