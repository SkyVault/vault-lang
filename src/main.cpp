#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <filesystem>

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
    std::cout << VAULT_PROMPT;
    const auto line = readInput();
    if (line == "q") break; 
    auto* progn = Vault::readCode(line);
    auto* result = Vault::eval(env, progn);
    std::cout << result << std::endl;
    deRef(progn);
  }

  return Vault::Status::SUCCESS;
}

void runScript(Obj* env, const std::string& path) { 
  auto code = readFile(path);
  auto* progn = Vault::readCode(code);
  auto* result = Vault::eval(env, progn); 
  std::cout << "\nreturned:\n" << result << std::endl;
}

using ArgsIter = std::vector<std::string>::iterator;

void newProject(ArgsIter it, ArgsIter end) {
  if (it == end) { std::cout << "Error: the new command expects an argument for the project name\n\tEx: vault new my-new-project\n"; return; }
  it += 1;
  if (it == end) { std::cout << "Error: the new command expects an argument for the project name\n\tEx: vault new my-new-project\n"; return; }
  std::string projectName = *it; 

  std::filesystem::create_directory(projectName); 
  std::ofstream projectFile(projectName + "/" + projectName + ".conf.vlt");
  std::ofstream entryFile(projectName + "/" + projectName + ".vlt");
  projectFile << "";
  entryFile << "";
  projectFile.close();
  entryFile.close();
}

int main(const int num_args, const char* args[]) { 
  if (num_args == 1) { return repl(); }

  std::vector<std::string> pargs;
  for (int i = 0; i < num_args; i++)
    pargs.emplace_back(std::string{args[i]});

  if (pargs[1] == "new") {
    newProject(pargs.begin() + 1, pargs.end());
  } else { 
    auto* env = newStdEnv();
    runScript(env, pargs[1]); 
  }

  return EXIT_SUCCESS;
}