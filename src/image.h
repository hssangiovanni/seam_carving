#ifndef IMAGE_H
#define IMAGE_H

#include <string>

class Image {
private:
    int width;
    int height;
    int n_channels;

    unsigned char* image_ptr;
    bool img_ptr_created_with_new;

    // Uses the average value of all channels
    void _convert_to_single_channel();
    double _apply_kernel_to_pixel(int x, int y, int channel, int kernel_size, double* kernel);

public:
    Image(std::string filepath, int required_channels = 3);

    int get_width();
    int get_height();
    unsigned char* get_image_ptr();
    int get_channels();


    void displayInfo();

    void convert_to_gray_scale();

    void convolute(int kernel_size, double* kernel);

    void zoom_in(int sx, int sy);

    void zoom_out(int sx, int sy);

    void rotate_90(bool clockwise = true);
    
    void save(std::string filepath);

    ~Image();
};

#endif