#include <iostream>
#include <sstream>
#include <string>

#include "vault.hpp"
#include "gc.hpp" 
#include "reader.hpp"
#include "object.hpp"

std::string readInput() {
  std::string result{""}; 
  std::getline(std::cin, result); 
  return result;
}

Vault::Status repl() {
  std::cout << "Vault (" << VAULT_VERSION << ")\n";

  Vault::Obj* progn = Vault::readCode("(+ 1 2 3)");

  deRef(progn); 

  return Vault::Status::SUCCESS;
}

int main(const int num_args, const char* args[]) {
  if (num_args == 1) { return repl(); }

  return EXIT_SUCCESS;
}