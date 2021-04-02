#include <iostream>
#include <sstream>
#include <string>

#include "vault.hpp"

std::string readInput() {
  std::string result{""}; 
  std::getline(std::cin, result); 
  return result;
}

Vault::Status repl() {
  std::cout << "Vault (" << VAULT_VERSION << ")\n";

  bool running = true;
  while (running) { 
    // Read
    std::cout << VAULT_PROMPT;

    std::string code = readInput();

    // Eval

    // Print
  }

  return Vault::Status::SUCCESS;
}

int main(const int num_args, const char* args[]) {
  if (num_args == 1) { return repl(); }

  return EXIT_SUCCESS;
}