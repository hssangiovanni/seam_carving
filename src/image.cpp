#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "image.h"

#include <iostream>
#include <cstring>
#include <cmath>

using namespace std;



// uses the avg value of all channels
void Image::_convert_to_single_channel(){
    unsigned char* single_channel_ptr = new unsigned char[width * height];

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int pixel_idx = (width * y + x) * n_channels;
            
            int channels_value_sum = 0;
            for (int i = 0; i<n_channels; i++){
                channels_value_sum += image_ptr[pixel_idx + i];
            }
            int avg_channels_value = channels_value_sum / n_channels;

            int single_channel_idx = width * y + x;
            single_channel_ptr[single_channel_idx] = avg_channels_value;
        }
    }
    stbi_image_free(image_ptr);
    image_ptr = single_channel_ptr;        

    n_channels = 1;
    img_ptr_created_with_new = true;
}
Image::Image(string filepath, int required_channels){
    img_ptr_created_with_new = false;
    image_ptr = stbi_load(filepath.c_str(), &width, &height, &n_channels, required_channels);
    if (image_ptr == nullptr)
        cout << "Error loading image at " << filepath << "\n";
}

int Image::get_width(){
    return width;
}
int Image::get_height(){
    return height;
}
unsigned char* Image::get_image_ptr(){
    return image_ptr;
}
int Image::get_channels() {
    return n_channels;
}

void Image::displayInfo(){
    cout << "---------------------\n";
    cout << "Width: " << width << "\nHeight: " << height << "\nNumber of n_channels: " << n_channels << "\n";
    cout << "---------------------\n";
    
}


void Image::convert_to_gray_scale(){
    if (n_channels == 1){
        cout << "WARNING: image already in grayscale";
        return;
    }
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int pixel_idx = (width * y + x) * n_channels;
            unsigned char r = image_ptr[pixel_idx];
            unsigned char g = image_ptr[pixel_idx + 1];
            unsigned char b = image_ptr[pixel_idx + 2];

            int luminance = round(0.299 * r + 0.587 * g + 0.114 * b);
            image_ptr[pixel_idx] = luminance;
            image_ptr[pixel_idx + 1] = luminance;
            image_ptr[pixel_idx + 2] = luminance;
        }
    }
    _convert_to_single_channel();
}

double Image::_apply_kernel_to_pixel(int x, int y, int channel, int kernel_size, double* kernel){
    int offset = kernel_size / 2;
    double result = 0.0;

    for (int ky = 0; ky < kernel_size; ky++) {
        for (int kx = 0; kx < kernel_size; kx++) {
            int image_y = y + ky - offset;
            int image_x = x + kx - offset;
            int image_idx =
                (image_y * width + image_x) * n_channels + channel;

            // Kernel rotated 180 degrees
            int kernel_y = kernel_size - 1 - ky;
            int kernel_x = kernel_size - 1 - kx;

            int kernel_idx =
                kernel_y * kernel_size + kernel_x;

            result += image_ptr[image_idx] * kernel[kernel_idx];
        }
    }

    return result;
}


void Image::zoom_out(int sx, int sy) {
    if (sx <= 0 || sy <= 0) {
        cout << "ERROR (zoom_out): sx and sy scaling factors must be positive integers\n";
        return;
    }

    int new_width = (width + sx - 1) / sx; //+sx -1 to avoid pixels falling in the "limbo"
    int new_height = (height + sy - 1) / sy; //+sy -1 to avoid pixels falling in the "limbo"
    int new_size = new_width * new_height * n_channels;

    unsigned char* aux = new unsigned char[new_size]();

    for (int out_y = 0; out_y < new_height; out_y++) {
        for (int out_x = 0; out_x < new_width; out_x++) {
            int start_x = out_x * sx;
            int start_y = out_y * sy;
            int end_x = min(start_x + sx, width);
            int end_y = min(start_y + sy, height);

            int pixel_count = (end_x - start_x) * (end_y - start_y); //area of rectangle

            for (int c = 0; c < n_channels; c++) {
                double sum = 0.0;
                for (int y = start_y; y < end_y; y++) {
                    for (int x = start_x; x < end_x; x++) {
                        int idx = (y * width + x) * n_channels + c;
                        sum += image_ptr[idx];
                    }
                }

                double avg = sum / pixel_count;
                int out_idx = (out_y * new_width + out_x) * n_channels + c;
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
}

void Image::zoom_in(int sx, int sy) {
    if (sx <= 0 || sy <= 0) {
        cout << "ERROR (zoom_in): sx and sy scaling factors must be positive integers\n";
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
                        int in_idx = (y * width + x) * n_channels + c;
                        int out_idx = (out_y * new_width + out_x) * n_channels + c;

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
}

void Image::convolute(int kernel_size, double* kernel) {
    if (kernel_size <= 0 || kernel_size % 2 == 0) {
        cout << "ERROR (convolute): kernel size should be a positive odd number, so it can be centralized in each pixel\n";
        return;
    }

    //offset by kernel_size/2 to avoid borders with undefined convolution
    int offset = kernel_size / 2;
    int image_size = width * height * n_channels;

    // Auxiliary image, initialized with black pixels to account for undefined border values
    unsigned char* aux = new unsigned char[image_size]();

    for (int y = offset; y < height - offset; y++) { 
        for (int x = offset; x < width - offset; x++) {
            for (int c = 0; c < n_channels; c++) {
                double result = _apply_kernel_to_pixel(x, y, c, kernel_size, kernel);

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
                int in_idx = (y * width + x) * n_channels + c;
                int out_idx = (out_y * new_width + out_x) * n_channels + c;

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
}

void Image::save(string filepath){
    int sucess = stbi_write_jpg(filepath.c_str(), width, height, n_channels, image_ptr, 90);
    cout << "Successfully saved image at filepath " << filepath << "\n";
    if (!sucess)
        cout << "Error saving image at filepath " << filepath << "\n";
}

Image::~Image() {
    if (img_ptr_created_with_new)
        delete[] image_ptr;
    else
        stbi_image_free(image_ptr);
}