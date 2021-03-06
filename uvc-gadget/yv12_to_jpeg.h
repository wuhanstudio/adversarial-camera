#include <stdint.h>
#include <malloc.h>
#include <jpeglib.h>

long unsigned int yv12_to_jpeg(const uint8_t* input, const int width, const int height, uint8_t* &outbuffer) {
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    long unsigned int outlen = 0;

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

    uint8_t* tmprowbuf = (uint8_t*) malloc( sizeof(uint8_t) * (width * 3) );

    JSAMPROW row_pointer[1];
    row_pointer[0] = &tmprowbuf[0];

    int u_offset = cinfo.image_width * cinfo.image_height;
    int v_offset = u_offset + (cinfo.image_width / 2) * (cinfo.image_height / 2);

    while (cinfo.next_scanline < cinfo.image_height) {
        unsigned i, j;
        for (i = 0, j = 0; i < cinfo.image_width; i += 1, j += 3) { 
            // input strides by 1 bytes, output strides by 3 (2 pixels)
            tmprowbuf[j + 0] = input[i + cinfo.image_width * cinfo.next_scanline];
            tmprowbuf[j + 1] = input[i / 2 + u_offset + (cinfo.image_width / 2) * (cinfo.next_scanline / 2)];
            tmprowbuf[j + 2] = input[i / 2 + v_offset + (cinfo.image_width / 2) * (cinfo.next_scanline / 2)];
        }
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    return outlen;
}
