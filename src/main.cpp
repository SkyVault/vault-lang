#include <iostream>
#include <sstream>
#include <string>

#include "vault.hpp"
#include "gc.hpp" 
#include "reader.hpp"
#include "object.hpp"
#include "eval.hpp"
#include "prelude.hpp"

using namespace Vault;

Vault::Status repl() {
  std::cout << "Vault (" << VAULT_VERSION << ")\n";

  auto* env = Vault::newStdEnv(); 

  while (true) {
    std::cout << "> ";
    const auto line = readInput();
    if (line == "q") break; 
    if (line == "(quit)") break;
    auto* progn = Vault::readCode(line);
    auto* result = Vault::eval(env, progn);
    std::cout << result->get(0) << std::endl;
    deRef(progn);
  }

  return Vault::Status::SUCCESS;
}

int main(const int num_args, const char* args[]) { 
  if (num_args == 1) { return repl(); }

  return EXIT_SUCCESS;
}