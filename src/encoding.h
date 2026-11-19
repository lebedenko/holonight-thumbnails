#pragma once

#include <QImage>

#include <algorithm>

namespace HolonightThumbnails::detail {

// Compare stored pixels without converting color, alpha or premultiplication.
template <typename Cancelled>
bool adaptiveNonuniform(const QImage& image, Cancelled cancelled) {
  switch (image.format()) {
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
    case QImage::Format_RGBX8888:
    case QImage::Format_RGBA8888:
    case QImage::Format_RGBA8888_Premultiplied:
      break;
    default:
      return false;
  }
  if (image.width() < 2 || image.height() < 2) {
    return false;
  }
  const int nx = std::min(32, image.width() - 1);
  const int ny = std::min(32, image.height() - 1);
  int count = 0;
  for (int j = 0; j < ny; ++j) {
    if (cancelled()) {
      return false;
    }
    const int y = 1 + ((2 * j + 1) * (image.height() - 1)) / (2 * ny);
    const auto* row = reinterpret_cast<const quint32*>(image.constScanLine(y));
    const auto* upper = reinterpret_cast<const quint32*>(image.constScanLine(y - 1));
    for (int i = 0; i < nx; ++i) {
      const int x = 1 + ((2 * i + 1) * (image.width() - 1)) / (2 * nx);
      count += row[x] != row[x - 1] && row[x] != upper[x];
    }
  }
  return 4 * count >= nx * ny;
}

}  // namespace HolonightThumbnails::detail
