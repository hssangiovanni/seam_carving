
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "image.h"

#include <iostream>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <stdexcept>

using namespace std;


// Extracts grayscale luminance already written into the first channel.
void Image::_convert_to_single_channel() {
    unsigned char* single_channel_ptr = new unsigned char[width * height];

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int i = y * width + x;
            single_channel_ptr[i] = image_ptr[i * n_channels];
        }
    }

    if (img_ptr_created_with_new)
        delete[] image_ptr;
    else
        stbi_image_free(image_ptr);

    image_ptr = single_channel_ptr;
    n_channels = 1;
    img_ptr_created_with_new = true;
    energy_map.clear();
}


Image::Image(string filepath, int required_channels)
    : width(0), height(0), n_channels(0), image_ptr(nullptr),
      img_ptr_created_with_new(false) {

    int file_channels = 0;

    image_ptr = stbi_load(
        filepath.c_str(),
        &width,
        &height,
        &file_channels,
        required_channels
    );

    if (!image_ptr)
        throw runtime_error("Error loading image at " + filepath);

    // stb_image reports the source channel count even when
    // forcing output channels.
    n_channels = required_channels != 0
               ? required_channels
               : file_channels;
}


int Image::get_width() const {
    return width;
}

int Image::get_height() const {
    return height;
}

int Image::get_channels() const {
    return n_channels;
}

unsigned char* Image::get_image_ptr() {
    return image_ptr;
}

const unsigned char* Image::get_image_ptr() const {
    return image_ptr;
}


void Image::displayInfo() const {
    cout << "---------------------\n";
    cout << "Width: " << width
         << "\nHeight: " << height
         << "\nNumber of channels: " << n_channels << "\n";
    cout << "---------------------\n";
}


void Image::convert_to_gray_scale() {
    if (n_channels == 1) {
        cout << "WARNING: image already in grayscale\n";
        return;
    }

    // Grayscale + alpha
    if (n_channels == 2) {
        _convert_to_single_channel();
        return;
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * n_channels;

            unsigned char r = image_ptr[idx];
            unsigned char g = image_ptr[idx + 1];
            unsigned char b = image_ptr[idx + 2];

            int luminance = static_cast<int>(
                round(0.299 * r + 0.587 * g + 0.114 * b)
            );

            image_ptr[idx] = static_cast<unsigned char>(luminance);
        }
    }

    _convert_to_single_channel();
}


// Applies a convolution kernel to an individual pixel.
double Image::_apply_kernel_to_pixel(
    int x,
    int y,
    int channel,
    int kernel_size,
    const double* kernel
) const {
    int offset = kernel_size / 2;
    double result = 0.0;

    for (int ky = 0; ky < kernel_size; ky++) {
        for (int kx = 0; kx < kernel_size; kx++) {

            // Replicate nearest border pixel
            int image_y = clamp(y + ky - offset, 0, height - 1);
            int image_x = clamp(x + kx - offset, 0, width - 1);

            int image_idx =
                (image_y * width + image_x) * n_channels + channel;

            // Rotate kernel 180 degrees for true convolution
            int kernel_y = kernel_size - 1 - ky;
            int kernel_x = kernel_size - 1 - kx;

            int kernel_idx = kernel_y * kernel_size + kernel_x;

            result += image_ptr[image_idx] * kernel[kernel_idx];
        }
    }

    return result;
}


void Image::zoom_out(int sx, int sy) {
    if (sx <= 0 || sy <= 0) {
        cout << "ERROR (zoom_out): sx and sy must be positive\n";
        return;
    }

    int new_width = (width + sx - 1) / sx;
    int new_height = (height + sy - 1) / sy;
    int new_size = new_width * new_height * n_channels;

    unsigned char* aux = new unsigned char[new_size]();

    for (int out_y = 0; out_y < new_height; out_y++) {
        for (int out_x = 0; out_x < new_width; out_x++) {

            int start_x = out_x * sx;
            int start_y = out_y * sy;

            int end_x = min(start_x + sx, width);
            int end_y = min(start_y + sy, height);

            int pixel_count =
                (end_x - start_x) * (end_y - start_y);

            for (int c = 0; c < n_channels; c++) {
                double sum = 0.0;

                for (int y = start_y; y < end_y; y++) {
                    for (int x = start_x; x < end_x; x++) {
                        int idx = (y * width + x) * n_channels + c;
                        sum += image_ptr[idx];
                    }
                }

                double avg = sum / pixel_count;

                int out_idx =
                    (out_y * new_width + out_x) * n_channels + c;

                aux[out_idx] = static_cast<unsigned char>(avg);
            }
        }
    }

    if (img_ptr_created_with_new)
        delete[] image_ptr;
    else
        stbi_image_free(image_ptr);

    image_ptr = aux;
    width = new_width;
    height = new_height;
    img_ptr_created_with_new = true;

    energy_map.clear();
}


void Image::zoom_in(int sx, int sy) {
    if (sx <= 0 || sy <= 0) {
        cout << "ERROR (zoom_in): sx and sy must be positive\n";
        return;
    }

    int new_width = width * sx;
    int new_height = height * sy;
    int new_size = new_width * new_height * n_channels;

    unsigned char* aux = new unsigned char[new_size]();

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            for (int dy = 0; dy < sy; dy++) {
                for (int dx = 0; dx < sx; dx++) {

                    int out_x = x * sx + dx;
                    int out_y = y * sy + dy;

                    for (int c = 0; c < n_channels; c++) {

                        int in_idx =
                            (y * width + x) * n_channels + c;

                        int out_idx =
                            (out_y * new_width + out_x) * n_channels + c;

                        aux[out_idx] = image_ptr[in_idx];
                    }
                }
            }
        }
    }

    if (img_ptr_created_with_new)
        delete[] image_ptr;
    else
        stbi_image_free(image_ptr);

    image_ptr = aux;
    width = new_width;
    height = new_height;
    img_ptr_created_with_new = true;

    energy_map.clear();
}


void Image::convolute(int kernel_size, const double* kernel) {
    if (kernel_size <= 0 || kernel_size % 2 == 0) {
        cout << "ERROR (convolute): kernel size must be positive and odd\n";
        return;
    }

    int offset = kernel_size / 2;
    int image_size = width * height * n_channels;

    // Initialize border pixels to black
    unsigned char* aux = new unsigned char[image_size]();

    for (int y = offset; y < height - offset; y++) {
        for (int x = offset; x < width - offset; x++) {
            for (int c = 0; c < n_channels; c++) {

                double result =
                    _apply_kernel_to_pixel(x, y, c, kernel_size, kernel);

                if (result < 0)
                    result = 0;
                else if (result > 255)
                    result = 255;

                int pixel_idx =
                    (y * width + x) * n_channels + c;

                aux[pixel_idx] =
                    static_cast<unsigned char>(result);
            }
        }
    }

    if (img_ptr_created_with_new)
        delete[] image_ptr;
    else
        stbi_image_free(image_ptr);

    image_ptr = aux;
    img_ptr_created_with_new = true;

    energy_map.clear();
}


// Calculate and store the Sobel energy map.
// Does NOT modify image_ptr.
void Image::calculate_energy() {
    static const double sobel_x[9] = {
        -1, 0, 1,
        -2, 0, 2,
        -1, 0, 1
    };

    static const double sobel_y[9] = {
        -1, -2, -1,
         0,  0,  0,
         1,  2,  1
    };

    // One energy value per pixel
    energy_map.assign(width * height, 0.0);

    // Ignore alpha for grayscale+alpha and RGBA images
    int color_channels =
        (n_channels == 2 || n_channels == 4)
        ? n_channels - 1
        : n_channels;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {

            double pixel_energy = 0.0;

            for (int c = 0; c < color_channels; c++) {

                double gx = _apply_kernel_to_pixel(
                    x, y, c, 3, sobel_x
                );

                double gy = _apply_kernel_to_pixel(
                    x, y, c, 3, sobel_y
                );

                pixel_energy += abs(gx) + abs(gy);
            }

            energy_map[y * width + x] = pixel_energy;
        }
    }
}


// Return the stored energy map without copying it.
const vector<double>& Image::get_energy_map() const {
    return energy_map;
}


// Save the energy map as a normalized grayscale PNG.
// Does NOT modify the original image or the energy map.
void Image::save_energy_map(const string& filepath) const {
    if (energy_map.empty()) {
        cerr << "ERROR: No energy map. "
             << "Call calculate_energy() first.\n";
        return;
    }

    double max_energy = *max_element(
        energy_map.begin(),
        energy_map.end()
    );

    vector<unsigned char> output(energy_map.size(), 0);

    if (max_energy > 0) {
        for (size_t i = 0; i < energy_map.size(); ++i) {

            double value = round(
                255.0 * energy_map[i] / max_energy
            );

            output[i] = static_cast<unsigned char>(
                clamp(value, 0.0, 255.0)
            );
        }
    }

    if (!stbi_write_png(
        filepath.c_str(),
        width,
        height,
        1,
        output.data(),
        width
    )) {
        cerr << "ERROR: Could not save energy map at "
             << filepath << "\n";
    } else {
        cout << "Saved energy map at " << filepath << "\n";
    }
}


void Image::rotate_90(bool clockwise) {
    int new_width = height;
    int new_height = width;
    int image_size = new_width * new_height * n_channels;

    unsigned char* aux = new unsigned char[image_size]();

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {

            int out_x, out_y;

            if (clockwise) {
                out_x = height - 1 - y;
                out_y = x;
            } else {
                out_x = y;
                out_y = width - 1 - x;
            }

            for (int c = 0; c < n_channels; c++) {

                int in_idx =
                    (y * width + x) * n_channels + c;

                int out_idx =
                    (out_y * new_width + out_x) * n_channels + c;

                aux[out_idx] = image_ptr[in_idx];
            }
        }
    }

    if (img_ptr_created_with_new)
        delete[] image_ptr;
    else
        stbi_image_free(image_ptr);

    image_ptr = aux;
    width = new_width;
    height = new_height;
    img_ptr_created_with_new = true;

    energy_map.clear();
}


void Image::save(string filepath) {
    int success = stbi_write_jpg(
        filepath.c_str(),
        width,
        height,
        n_channels,
        image_ptr,
        90
    );

    if (success)
        cout << "Successfully saved image at filepath "
             << filepath << "\n";
    else
        cerr << "Error saving image at filepath "
             << filepath << "\n";
}


Image::~Image() {
    if (img_ptr_created_with_new)
        delete[] image_ptr;
    else
        stbi_image_free(image_ptr);
}
