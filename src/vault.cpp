#include "vault.hpp"

std::string readFile(const std::string& path) {
  auto f = std::ifstream(path);
  if (!f) {
    std::cout << "Cannot find file '" << path << std::endl;
    std::exit(EXIT_FAILURE);
  }
  return std::string(
    std::istreambuf_iterator<char>(f),
    std::istreambuf_iterator<char>()
  );
} 