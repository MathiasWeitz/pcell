#include <cmath>
#include <iostream>
#include <png.h>

#include "pngIO.h"

using namespace std;

PNGRead::PNGRead(const char *fileName) : filename(fileName) {};
PNGRead::~PNGRead() {
  if (png != nullptr) {
    // int width = png_get_image_width(png, info);
    int height = png_get_image_height(png, info);

    // Clean up
    for (int y = 0; y < height; y++) {
      free(row_pointers[y]);
    }
    free(row_pointers);
    png_destroy_read_struct(&png, &info, NULL);
    png = nullptr;
    info = nullptr;
  }
}

void PNGRead::read() {
  if (png == nullptr) {
    // load the file
    // cout << "read Image" << endl;
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
      perror("Error opening file");
      return;
    }

    png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
      fclose(fp);
      return;
    }

    // Create the info struct
    info = png_create_info_struct(png);
    if (!info) {
      png_destroy_read_struct(&png, NULL, NULL);
      fclose(fp);
      return;
    }

    // Setup error handling (required by libpng)
    if (setjmp(png_jmpbuf(png))) {
      png_destroy_read_struct(&png, &info, NULL);
      fclose(fp);
      return;
    }

    // Initialize PNG reading
    png_init_io(png, fp);

    // Read PNG header info
    png_read_info(png, info);

    // Get image details
    // int width = png_get_image_width(png, info);
    int height = png_get_image_height(png, info);
    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);
    // int channels = png_get_channels(png, info);

    // cout << "getWidth:\t" << width << endl;
    // cout << "getHeight:\t" << height << endl;

    // Set up transformations (expand palette, transparency, etc.)
    if (bit_depth == 16) png_set_strip_16(png);

    if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png);

    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_expand_gray_1_2_4_to_8(png);

    if (png_get_valid(png, info, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png);

    // Update the png info after applying transformations
    png_read_update_info(png, info);

    // Allocate memory to hold image data
    row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
    for (int y = 0; y < height; y++) {
      row_pointers[y] = (png_byte *)malloc(png_get_rowbytes(png, info));
    }

    // Read the image into memory
    png_read_image(png, row_pointers);

    fclose(fp);
  }
}

int PNGRead::getHeight() {
  read();
  return png_get_image_height(png, info);
}
int PNGRead::getWidth() {
  read();
  return png_get_image_width(png, info);
}
std::tuple<int, int, int> PNGRead::get(int y, int x) {
  read();

  png_bytep row = row_pointers[y];
  png_bytep px = &(row[x * 3]);  // Each pixel is 3 bytes (RGB)

  // Access the RGB values
  //
  png_byte r = px[0];  // Red
  png_byte g = px[1];  // Green
  png_byte b = px[2];  // Blue

  return std::make_tuple(r, g, b);
}

/*************************************************************************
 *        Write
 *************************************************************************/

PNGWrite::PNGWrite(char *fileName) : filename(fileName) {};
PNGWrite::~PNGWrite() {
  if (row_pointers != nullptr) {
    free(row_pointers);
    row_pointers = nullptr;
  }
  if (image_data != nullptr) {
    free(image_data);
    image_data = nullptr;
  }

  if (png) {
    png_destroy_write_struct(&png, NULL);
    png = nullptr;
  }
};

void PNGWrite::setHeight(int h) { height = h; };
void PNGWrite::setWidth(int w) { width = w; };

void PNGWrite::init() {
  if (width > 0 && height > 0 && png == nullptr) {
#if 0
    png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
      return;
    }

    // Initialize the PNG info structure
    info = png_create_info_struct(png);
    if (!info) {
      png_destroy_write_struct(&png, NULL);
      png = nullptr;
      return;
    }

    // Set up error handling
    if (setjmp(png_jmpbuf(png))) {
      png_destroy_write_struct(&png, &info);
      png = nullptr;
      info = nullptr;
      return;
    }

    // Set the image properties
    // bit_depth        1, 2, 4, 8, 16
    // color_type       PNG_COLOR_TYPE_GRAY, PNG_COLOR_TYPE_GRAY_ALPHA,
    // PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB, PNG_COLOR_TYPE_RGB_ALPHA
    // interlace_type   PNG_INTERLACE_NONE, PNG_INTERLACE_ADAM7
    // compression_type PNG_COMPRESSION_TYPE_DEFAULT,
    // filter_method    PNG_FILTER_TYPE_DEFAULT
    png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);
#endif

    image_data = (png_bytep)malloc(3 * width * height * sizeof(png_byte));
    row_pointers = (png_bytep *)malloc(height * sizeof(png_bytep));

    for (int y = 0; y < height; y++) {
      row_pointers[y] = image_data + y * width * 3;
    }

    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        int pos = (y * width + x) * 3;
        image_data[pos] = 128;      // Red
        image_data[pos + 1] = 128;  // Green
        image_data[pos + 2] = 128;  // Blue
      }
    }
  }
};

void PNGWrite::write() {
  if (image_data != nullptr) {
    png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
      png_destroy_write_struct(&png, &info);
      return;
    }
    info = png_create_info_struct(png);
    if (!info) {
      png_destroy_write_struct(&png, NULL);
      png = nullptr;
      return;
    }
    // Set up error handling
    if (setjmp(png_jmpbuf(png))) {
      png_destroy_write_struct(&png, &info);
      png = nullptr;
      info = nullptr;
      return;
    }
    png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
      perror("Error opening file");
      png_destroy_write_struct(&png, &info);
      return;
    }
    png_init_io(png, fp);
    png_write_info(png, info);
    png_write_image(png, row_pointers);
    png_write_end(png, NULL);
    fclose(fp);
    png_destroy_write_struct(&png, &info);
  }
};

void PNGWrite::set(int y, int x, int r, int g, int b) {
  if (image_data == nullptr) {
    init();
  }
  if (image_data != nullptr) {
    unsigned index = (y * width + x) * 3;
    image_data[index + 0] = r;  // Red
    image_data[index + 1] = g;  // Green
    image_data[index + 2] = b;  // Blue
  }
};
