#ifndef IMAGE_H
#define IMAGE_H

#include <string>
#include <vector>

class Image {
private:
    int width;
    int height;
    int n_channels;

    unsigned char* image_ptr;
    bool img_ptr_created_with_new;

    // One energy value per pixel
    std::vector<double> energy_map;

    void _convert_to_single_channel();

    double _apply_kernel_to_pixel( int x, int y, int channel, int kernel_size, const double* kernel) const;

    // Seam carving auxiliary methods
    double _shortest_path(int x, int y, std::vector<std::vector<double>>& M) const;

    void _remove_vertical_seam(const std::vector<int>& seam);

public:
    Image(std::string filepath, int required_channels = 3);

    // Prevent unsafe shallow copies
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    int get_width() const;
    int get_height() const;
    int get_channels() const;

    unsigned char* get_image_ptr();
    const unsigned char* get_image_ptr() const;

    void displayInfo() const;

    void convert_to_gray_scale();

    void convolute(int kernel_size, const double* kernel);

    // Energy map operations
    void calculate_energy();

    const std::vector<double>& get_energy_map() const;

    double get_pixel_energy(int x, int y);

    void save_energy_map(const std::string& filepath) const;

    void apply_seam_carving(int n);

    void zoom_in(int sx, int sy);
    void zoom_out(int sx, int sy);

    void rotate_90(bool clockwise = true);

    void save(std::string filepath);

    ~Image();
};

#endif