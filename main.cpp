#include "source/core.hpp"

#include <cstdlib>
#include <cstring>

namespace {
// Accepts "2", "--level 2", or "--level=2"; anything else keeps level 1.
int parseStartLevel(int argc, char const *argv[]) {
  for (int index = 1; index < argc; index++) {
    std::string argument = argv[index];
    std::string value;

    if (argument.rfind("--level=", 0) == 0) {
      value = argument.substr(8);
    } else if (argument == "--level" || argument == "-l") {
      if (index + 1 >= argc) {
        std::cerr << "missing value for " << argument << std::endl;
        return 1;
      }
      value = argv[++index];
    } else if (argument == "--help" || argument == "-h") {
      std::cout << "usage: Mario [--level N]   (N is 1, 2, or 3)" << std::endl;
      exit(0);
    } else {
      value = argument;
    }

    int level = atoi(value.c_str());
    if (level < 1 || level > 3) {
      std::cerr << "level must be 1, 2, or 3; starting on level 1" << std::endl;
      return 1;
    }
    return level;
  }
  return 1;
}
}  // namespace

int main(int argc, char const *argv[]) {
  srand(time(NULL));
  Core core(parseStartLevel(argc, argv));
  try {
    core.loop();
  } catch (std::string exception) {
    std::cerr << "EXCEPTION: " << exception << std::endl;
    exit(1);
  }

  return 0;
}
