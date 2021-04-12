#include "vault.hpp"

std::string readFile(const std::string& path) {
  auto f = std::ifstream(path);
  return std::string(
    std::istreambuf_iterator<char>(f),
    std::istreambuf_iterator<char>()
  );
} 