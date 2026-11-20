#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>

#include "image.h"

using namespace std;




int main(){
    Image image("img/Space_187k.jpg");
    image.displayInfo();
    image.save("teste.jpg");
    return 1;
}