#include "../src/encoding.h"

#include <gtest/gtest.h>

using HolonightThumbnails::detail::adaptiveNonuniform;

namespace {
constexpr auto running = [] { return false; };
}  // namespace

TEST(Encoding, SupportedFormatsIgnorePaddingAndDetectStoredPixelDifferences) {
  for (auto format : {QImage::Format_RGB32, QImage::Format_ARGB32, QImage::Format_ARGB32_Premultiplied,
                      QImage::Format_RGBX8888, QImage::Format_RGBA8888, QImage::Format_RGBA8888_Premultiplied}) {
    SCOPED_TRACE(static_cast<int>(format));
    alignas(4) unsigned char bytes[32] = {};
    QImage image(bytes, 2, 2, 16, format);
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
      for (int y = 0; y < 33; ++y) {
        for (int x = 0; x < 33; ++x) {
          const int v = pattern == 0   ? 20
                        : pattern == 1 ? x * 7
                        : pattern == 2 ? (x % 2) * 200
                        : pattern == 3 ? (y % 2) * 200
                        : pattern == 4 ? (x > 8 && x < 24 && y > 8 && y < 24 ? 100 : 20)
                        : pattern == 5 ? (x + y) % 2 * 200
                                       : (x * 13 + y * 7) % 256;
          sample.setPixelColor(x, y, QColor(v, v, v, 128));
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
  for (int n = 0; n < 255; ++n) {
    image.setPixel(2 + 2 * (n % 32), 2 + 2 * (n / 32), qRgb(255, 255, 255));
  }
  EXPECT_FALSE(adaptiveNonuniform(image, running));
  image.setPixel(64, 16, qRgb(255, 255, 255));
  EXPECT_TRUE(adaptiveNonuniform(image, running));
  image.setPixel(2, 18, qRgb(255, 255, 255));
  EXPECT_TRUE(adaptiveNonuniform(image, running));
  // Odd coordinates are unsampled. A shifted grid would select these marks.
  image.fill(Qt::black);
  for (int y = 1; y < 65; y += 2) {
    for (int x = 1; x < 65; x += 2) {
      image.setPixel(x, y, qRgb(255, 255, 255));
    }
  }
  EXPECT_FALSE(adaptiveNonuniform(image, running));
}

TEST(Encoding, ScansEntireGridAndCancelsBetweenRows) {
  QImage image(65, 65, QImage::Format_RGB32);
  for (int y = 0; y < 65; ++y) {
    for (int x = 0; x < 65; ++x) {
      image.setPixel(x, y, qRgb((x + y) % 2 * 255, 0, 0));
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
