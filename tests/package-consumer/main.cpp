#include <holonight_thumbnails/cache.h>

int main() {
  return HolonightThumbnails::tierForSize({129, 100}) == 256 ? 0 : 1;
}
