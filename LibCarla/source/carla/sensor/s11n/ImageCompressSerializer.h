// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/sensor/RawData.h"

#include <iostream>
#include <cstdint>
#include <cstring>
#include <jpeglib.h>
#include <setjmp.h>
#include <chrono>

#define DEFAULT_JPEG_QUALITY 85
#define DEFAULT_JPEG_BUFFER_SIZE (4*1024*1024)   // Uncompressed 1920x1080 RGBA is 8 MB => 4 MB worst case
#define BGRA_BYTES_PER_PIXEL 4

namespace carla {
namespace sensor {

  class SensorData;

namespace s11n {

  /// Serializes image buffers generated by camera sensors.
  class ImageCompressSerializer {
  private:
      static void compress(unsigned char* input_buffer, unsigned int width, unsigned int height,
                           unsigned char* output_buffer, unsigned long* outsize, int quality);

  public:

#pragma pack(push, 1)
    struct CompressedImageHeader {
      uint32_t width;
      uint32_t height;
      float fov_angle;
      uint32_t size; // Compressed size
      int64_t timestamp;
    };
#pragma pack(pop)

    constexpr static auto header_offset = sizeof(CompressedImageHeader);

    static const CompressedImageHeader &DeserializeHeader(const RawData &data) {
      return *reinterpret_cast<const CompressedImageHeader *>(data.begin());
    }

    template <typename Sensor>
    static Buffer Serialize(const Sensor &sensor, Buffer &&bitmap);

    static SharedPtr<SensorData> Deserialize(RawData &&data);
  };

  template <typename Sensor>
  inline Buffer ImageCompressSerializer::Serialize(const Sensor &sensor, Buffer &&bitmap) {
    DEBUG_ASSERT(bitmap.size() > sizeof(CompressedImageHeader));
    // It looks like the data in "bitmap" (input) has
    // - 16-bytes of header
    // - the RGBA data after the header
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    Buffer compressedBuffer;  // For some reason, the constructor Buffer(size) does not compile with UE4
    compressedBuffer.resize(DEFAULT_JPEG_BUFFER_SIZE); // Reserve enough memory
    unsigned char* inputData = bitmap.data() + 16;
    unsigned char* outputBuffer = compressedBuffer.data() + header_offset;
    unsigned int width = sensor.GetImageWidth();
    unsigned int height = sensor.GetImageHeight();
    unsigned long jpeg_size = DEFAULT_JPEG_BUFFER_SIZE - header_offset;
    ImageCompressSerializer::compress(inputData, width, height, outputBuffer, &jpeg_size, DEFAULT_JPEG_QUALITY);
    CompressedImageHeader header = {
      sensor.GetImageWidth(),
      sensor.GetImageHeight(),
      sensor.GetFOVAngle(),
      static_cast<uint32_t>(jpeg_size),
      ms.count()
    };
    std::memcpy(compressedBuffer.data(), reinterpret_cast<const void *>(&header), sizeof(header));
    compressedBuffer.resize( header_offset + jpeg_size );
    return compressedBuffer;
  }

} // namespace s11n
} // namespace sensor
} // namespace carla
