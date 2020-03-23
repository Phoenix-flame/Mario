#include "source/core.hpp"


int main(int argc, char const *argv[]) {
  srand(time(NULL));
  Core core;
  try {
    core.loop();
  } catch (std::string exception) {
    std::cerr << "EXCEPTION: " << exception << std::endl;
    exit(1);
  }

  return 0;
}
