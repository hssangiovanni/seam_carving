#include "image.h"
#include <iostream>
#include <vector>
#include <cstdlib>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <columns_to_remove> <source_image> <output_filepath>\n";
        return 1;
    }

    int columns_to_remove = atoi(argv[1]);
    if (columns_to_remove <= 0) {
        cerr << "columns_to_remove must be a positive integer\n";
        return 1;
    }

    Image img(argv[2], 3);


    // Save energy map visualization
    img.save_energy_map("img/energy.png");


    img.apply_seam_carving(columns_to_remove);

    img.save(argv[3]);

    return 0;
}
