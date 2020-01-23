#include <clocale>
#include <cstdlib>
#include <iostream>

int main(void){
  if(!setlocale(LC_ALL, "")){
    std::cerr << "Couldn't set locale\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
