#include "../src/encoding.h"

#include <array>
#include <gtest/gtest.h>

using HolonightThumbnails::detail::adaptiveNonuniform;

namespace {
int patternValue(int pattern, int column, int line) {
  switch (pattern) {
    case 0:
      return 20;
    case 1:
      return column * 7;
    case 2:
      return (column % 2) * 200;
    case 3:
      return (line % 2) * 200;
    case 4:
      return column > 8 && column < 24 && line > 8 && line < 24 ? 100 : 20;
    case 5:
      return ((column + line) % 2) * 200;
    default:
      return ((column * 13) + (line * 7)) % 256;
  }
}
constexpr auto running = [] { return false; };
}  // namespace

TEST(Encoding, SupportedFormatsIgnorePaddingAndDetectStoredPixelDifferences) {
  for (auto format : {
           QImage::Format_RGB32,
           QImage::Format_ARGB32,
           QImage::Format_ARGB32_Premultiplied,
           QImage::Format_RGBX8888,
           QImage::Format_RGBA8888,
           QImage::Format_RGBA8888_Premultiplied,
       }) {
    SCOPED_TRACE(static_cast<int>(format));
    alignas(4) std::array<unsigned char, 32> bytes{};
    QImage image(bytes.data(), 2, 2, 16, format);
    image.fill(QColor(31, 83, 149, 128));
    bytes[8] = 17;
    bytes[24] = 99;
    EXPECT_FALSE(adaptiveNonuniform(image, running));
    bytes[20] ^= 1;
    EXPECT_TRUE(adaptiveNonuniform(image, running));
    EXPECT_FALSE(adaptiveNonuniform(image, [] { return true; }));
    QImage tiny(1, 33, format);
    EXPECT_FALSE(adaptiveNonuniform(tiny, running));
    EXPECT_FALSE(adaptiveNonuniform(tiny.transformed(QTransform().rotate(90)), running));
    for (int pattern = 0; pattern < 7; ++pattern) {
      QImage sample(33, 33, format);
      for (int line = 0; line < 33; ++line) {
        for (int column = 0; column < 33; ++column) {
          const int value = patternValue(pattern, column, line);
          sample.setPixelColor(column, line, QColor(value, value, value, 128));
        }
      }
      EXPECT_EQ(adaptiveNonuniform(sample, running), pattern >= 5);
    }
  }
}

TEST(Encoding, AlphaOnlyDifferencesAndUnsupportedInputs) {
  QImage alpha(2, 2, QImage::Format_RGBA8888);
  alpha.fill(QColor(1, 2, 3, 4));
  alpha.setPixelColor(1, 1, QColor(1, 2, 3, 5));
  EXPECT_TRUE(adaptiveNonuniform(alpha, running));
  EXPECT_FALSE(adaptiveNonuniform(alpha.convertToFormat(QImage::Format_Grayscale8), running));
  EXPECT_FALSE(adaptiveNonuniform(QImage(), running));
}

TEST(Encoding, ExactMidpointCoordinatesAndInclusiveQuarterBoundary) {
  // For 65x65 the sampled coordinates are 2,4,...,64. Isolated marks
  // at those coordinates differ from both neighbors without affecting other samples.
  QImage image(65, 65, QImage::Format_RGB32);
  image.fill(Qt::black);
  for (int index = 0; index < 255; ++index) {
    image.setPixel(2 + (2 * (index % 32)), 2 + (2 * (index / 32)), qRgb(255, 255, 255));
  }
  EXPECT_FALSE(adaptiveNonuniform(image, running));
  image.setPixel(64, 16, qRgb(255, 255, 255));
  EXPECT_TRUE(adaptiveNonuniform(image, running));
  image.setPixel(2, 18, qRgb(255, 255, 255));
  EXPECT_TRUE(adaptiveNonuniform(image, running));
  // Odd coordinates are unsampled. A shifted grid would select these marks.
  image.fill(Qt::black);
  for (int line = 1; line < 65; line += 2) {
    for (int column = 1; column < 65; column += 2) {
      image.setPixel(column, line, qRgb(255, 255, 255));
    }
  }
  EXPECT_FALSE(adaptiveNonuniform(image, running));
}

TEST(Encoding, ScansEntireGridAndCancelsBetweenRows) {
  QImage image(65, 65, QImage::Format_RGB32);
  for (int line = 0; line < 65; ++line) {
    for (int column = 0; column < 65; ++column) {
      image.setPixel(column, line, qRgb((column + line) % 2 * 255, 0, 0));
    }
  }
  int rows = 0;
  EXPECT_TRUE(adaptiveNonuniform(image, [&] {
    ++rows;
    return false;
  }));
  EXPECT_EQ(rows, 32);
  rows = 0;
  EXPECT_FALSE(adaptiveNonuniform(image, [&] { return ++rows == 17; }));
  EXPECT_EQ(rows, 17);
}
