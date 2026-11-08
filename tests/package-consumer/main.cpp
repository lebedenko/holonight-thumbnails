#include <holonight_thumbnails/cache.h>
#ifdef ISOLATED
#include <holonight_thumbnails/isolated.h>
#endif
int main() {
#ifdef ISOLATED
  const std::atomic_bool cancelled{true};
  const auto result = HolonightThumbnails::lookupIsolated({}, HolonightImages::IsolatedClient("/absent"), cancelled);
  if (result.runtime != HolonightImages::RuntimeStatus::Cancelled || result.image) {
    return 2;
  }
#endif
  return HolonightThumbnails::tierForSize({129, 100}) == 256 ? 0 : 1;
}
