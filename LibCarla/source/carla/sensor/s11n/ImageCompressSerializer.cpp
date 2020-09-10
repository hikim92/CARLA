// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/sensor/s11n/ImageCompressSerializer.h"

#include "carla/sensor/data/Image.h"

#include <jpeglib.h>

using namespace std::chrono;

namespace carla {
namespace sensor {
namespace s11n {

  void ImageCompressSerializer::compress(unsigned char* input_buffer, unsigned int width, unsigned int height,
            unsigned char* output_buffer, unsigned long* outsize, int quality)
  {
      struct jpeg_compress_struct cinfo;
      struct jpeg_error_mgr jerr;
      JSAMPROW row_pointer[1];  // pointer to JSAMPLE row[s]
      cinfo.err = jpeg_std_error(&jerr);
      jpeg_create_compress(&cinfo);
      jpeg_mem_dest(&cinfo, &output_buffer, outsize);
      cinfo.image_width      = width;         // image width and height, in pixels
      cinfo.image_height     = height;
      cinfo.input_components = BGRA_BYTES_PER_PIXEL;  // of color components per pixel
      cinfo.in_color_space   = JCS_EXT_BGRA;  // colorspace of input image
      jpeg_set_defaults(&cinfo);
      jpeg_set_quality(&cinfo, quality, TRUE); // limit to baseline-JPEG values
      jpeg_start_compress(&cinfo, TRUE);
      unsigned int row_stride = width * 4;    // JSAMPLEs per row in image_buffer
      while (cinfo.next_scanline < cinfo.image_height) {
          row_pointer[0] = & input_buffer[cinfo.next_scanline * row_stride];
          (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
      }
      jpeg_finish_compress(&cinfo);
      jpeg_destroy_compress(&cinfo);
  }

    struct my_error_mgr {
      struct jpeg_error_mgr pub;	/* "public" fields */
      jmp_buf setjmp_buffer;	    /* for return to caller */
    };

    typedef struct my_error_mgr* my_error_ptr;

    void my_error_exit (j_common_ptr cinfo)
    {
      my_error_ptr myerr = reinterpret_cast<my_error_ptr>(cinfo->err);
      (*cinfo->err->output_message) (cinfo);
      longjmp(myerr->setjmp_buffer, 1);
    }

    /* Will do the malloc on *image_buffer - should be NULL at start, and don't forget to free it */
    int uncompress( const unsigned char *inbuffer, const unsigned long insize,
                    unsigned char* outbuf, unsigned long* outsize,
                    unsigned int* width, unsigned int* height )
    {
      struct jpeg_decompress_struct cinfo;
      struct my_error_mgr jerr;
      JSAMPROW scanlines[1];
      unsigned long size = 0;
      unsigned int row_stride;
      cinfo.err = jpeg_std_error(&jerr.pub);
      jerr.pub.error_exit = my_error_exit;
      if (setjmp(jerr.setjmp_buffer)) {
        jpeg_destroy_decompress(&cinfo);
        return 0;
      }
      jpeg_create_decompress(&cinfo);
      jpeg_mem_src(&cinfo, inbuffer, insize);
      (void) jpeg_read_header(&cinfo, TRUE);
      cinfo.out_color_space = JCS_EXT_BGRA;
      (void) jpeg_start_decompress(&cinfo);
      row_stride = cinfo.output_width * 4;
      *width = cinfo.output_width;
      *height = cinfo.output_height;
      size = row_stride * cinfo.output_height;
      if(*outsize>=size) *outsize = size;
      else printf("OUTPUT BUFFER TOO SMALL\n");
      while (cinfo.output_scanline < cinfo.output_height) {
          scanlines[0] = outbuf + cinfo.output_scanline * row_stride;
          (void) jpeg_read_scanlines(&cinfo, scanlines, 1);
      }
      (void) jpeg_finish_decompress(&cinfo);
      jpeg_destroy_decompress(&cinfo);
      return 1;
    }

  SharedPtr<SensorData> ImageCompressSerializer::Deserialize(RawData &&data) {
      // Input : RawData :
      // - sensor header (timestamp, sensor type, transform, ..)
      // - the data that is directly accessible with data() or begin() (points after the header)
      // - size() excludes the sensor header (==end()-begin())
      // - data assumed to be the CompressedImageHeader + jpeg data
      // Output : Image :
      // - sensor header
      // - image header
      // - uncompressed image data
      using ImageHeader = s11n::ImageSerializer::ImageHeader;
      using SensorHeader = s11n::SensorHeaderSerializer::Header;
      CompressedImageHeader* pCompressedHeader = reinterpret_cast<CompressedImageHeader*>(data.data());
      size_t sensorHeaderSize = sizeof(SensorHeader);
      size_t imageHeaderSize = sizeof(ImageHeader);
      size_t imageDataSize = BGRA_BYTES_PER_PIXEL * pCompressedHeader->width * pCompressedHeader->height;
      Buffer fullBuffer( sensorHeaderSize + imageHeaderSize + imageDataSize );
      //std::cout << "Uncompressing buffer of " << data.size() << " bytes"
      //          << ", frame " << data.GetFrame()
      //          << ", timestamp "  << data.GetTimestamp()
      //          << ", " << pCompressedHeader->width << " x " << pCompressedHeader->height << " pixels"
      //          << ", jpeg of " << pCompressedHeader->size << " bytes" << std::endl;
      // Fill the Sensor header
      SensorHeader* pSensorHeader = reinterpret_cast<SensorHeader*>(&(fullBuffer[0]));
      pSensorHeader->sensor_type = data.GetSensorTypeId();
      pSensorHeader->timestamp = data.GetTimestamp();
      pSensorHeader->frame = data.GetFrame();
      pSensorHeader->sensor_transform = data.GetSensorTransform();
      // Fill the Image Header
      ImageHeader* pImageHeader = reinterpret_cast<ImageHeader*>(&(fullBuffer[sensorHeaderSize]));
      pImageHeader->width = pCompressedHeader->width;
      pImageHeader->height = pCompressedHeader->height;
      pImageHeader->fov_angle = pCompressedHeader->fov_angle;
      // Uncompress
      const unsigned char *jpegData = data.data() + sizeof(CompressedImageHeader);
      const unsigned long jpegDataSize = pCompressedHeader->size;
      unsigned long outSize = imageDataSize;
      unsigned char* pixelBuffer = &(fullBuffer[sensorHeaderSize+imageHeaderSize]);
      unsigned int uwidth, uheight;
      uncompress( jpegData, jpegDataSize, pixelBuffer, &outSize, &uwidth, &uheight );
      // DEBUG
      //if((pSensorHeader->frame % 1000) == 0) {
      //    milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
      //    std::cout << "Frame " << pSensorHeader->frame << " - end of decompression " << ms.count() << std::endl;
      //}
      // If everything is fine, (uwidth==pHeader->with) and (uheight==pHeader->height)
      RawData outputData(std::move(fullBuffer));  // Create this from the sensor header of data + Buffer imgOut
      auto image = SharedPtr<data::Image>(new data::Image{std::move(outputData)});
      // DEBUG
      //if((pSensorHeader->frame % 1000) == 0) {
      //    milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
      //    int64_t lag = ms.count() - pCompressedHeader->timestamp;
      //    std::cout << "Frame " << pSensorHeader->frame << " - lag (comp/rpc/decomp) : " << lag << std::endl;
      //}
      return image;
  }

} // namespace s11n
} // namespace sensor
} // namespace carla
