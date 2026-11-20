#include "image.h"
#include <iostream>
#include <vector>

using namespace std;

int main() {
    Image img("img/golfinho_2.jpg", 3);

    // Calculate and store energy
    img.calculate_energy();

    // Access the stored energy map
    const vector<double>& energy = img.get_energy_map();
    // or the energy values directly (e.g. pixel 10, 20)
    cout << img.get_pixel_energy(10, 20) << "\n";

    // Save energy map visualization
    img.save_energy_map("img/energy.png");


    img.apply_seam_carving(200);

    img.save("output.jpg");

    return 0;
}