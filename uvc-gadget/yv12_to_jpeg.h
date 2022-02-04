#include <stdint.h>
#include <malloc.h>
#include <jpeglib.h>

uint32_t yv12_to_jpeg(const uint8_t* input, const int width, const int height, uint8_t* &outbuffer) {
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    // uint8_t* outbuffer = NULL;
    uint32_t outlen = 0;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_mem_dest(&cinfo, &outbuffer, &outlen);

    // jrow is a libjpeg row of samples array of 1 row pointer
    cinfo.image_width = width & -1;
    cinfo.image_height = height & -1;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_YCbCr; //libJPEG expects YUV 3bytes, 24bit, YUYV --> YUV YUV YUV

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 92, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    uint8_t* tmprowbuf = malloc( sizeof(uint8_t) * (width * 3) );

    JSAMPROW row_pointer[1];
    row_pointer[0] = &tmprowbuf[0];
    while (cinfo.next_scanline < cinfo.image_height) {
        unsigned i, j;
        for (i = 0, j = 0; i < cinfo.image_width; i += 1, j += 3) { //input strides by 4 bytes, output strides by 6 (2 pixels)
            tmprowbuf[j + 0] = input[i + cinfo.image_width * cinfo.next_scanline]; // Y (unique to this pixel)
            tmprowbuf[j + 1] = input[i / 2 + cinfo.image_width * cinfo.image_height + (cinfo.image_width / 2) * (cinfo.next_scanline / 2)]; // U (shared between pixels)
            tmprowbuf[j + 2] = input[i / 2 + cinfo.image_width * cinfo.image_height + (cinfo.image_width / 2) * (cinfo.image_height / 2) + (cinfo.image_width / 2) * (cinfo.next_scanline / 2)]; // V (shared between pixels)
        }
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    return outlen;
}
