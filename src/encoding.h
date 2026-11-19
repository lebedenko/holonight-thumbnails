#pragma once

#include <QImage>

#include <algorithm>
#include <span>

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
  const int columns = std::min(32, image.width() - 1);
  const int rows = std::min(32, image.height() - 1);
  int count = 0;
  for (int j = 0; j < rows; ++j) {
    if (cancelled()) {
      return false;
    }
    const int line = 1 + ((((2 * j) + 1) * (image.height() - 1)) / (2 * rows));
    // Supported 32-bit formats provide aligned, contiguous stored pixels.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    const std::span row(reinterpret_cast<const quint32*>(image.constScanLine(line)), image.width());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    const std::span upper(reinterpret_cast<const quint32*>(image.constScanLine(line - 1)), image.width());
    for (int i = 0; i < columns; ++i) {
      const int column = 1 + ((((2 * i) + 1) * (image.width() - 1)) / (2 * columns));
      count += row[column] != row[column - 1] && row[column] != upper[column];
    }
  }
  return 4 * count >= columns * rows;
}

}  // namespace HolonightThumbnails::detail
