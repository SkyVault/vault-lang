#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <filesystem>

#include "gc.hpp" 
#include "vault.hpp"
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
    auto* result = Vault::eval(env, progn, true);
    std::cout << result << std::endl;
    Vault::Gc::markAndSweep(env);
  }

  Vault::Gc::sweep();
  return Vault::Status::SUCCESS;
}

void runScript(Obj* env, const std::string& path) { 
  auto code = readFile(path);
  auto* progn = Vault::readCode(code);
  putInEnv(env, newAtom("*source-code*"), progn);
  auto* result = Vault::eval(env, progn); 
  Vault::Gc::sweep();
}

using ArgsIter = std::vector<std::string>::iterator;

void newProject(ArgsIter it, ArgsIter end) {
  if (it == end) { std::cout << "Error: the new command expects an argument for the project name\n\tEx: vault new my-new-project\n"; return; }
  it += 1;
  if (it == end) { std::cout << "Error: the new command expects an argument for the project name\n\tEx: vault new my-new-project\n"; return; }
  std::string projectName = *it; 

  std::filesystem::create_directory(projectName); 
  std::ofstream projectFile(projectName + "/" + projectName + ".vlt-conf");
  std::ofstream entryFile(projectName + "/" + projectName + ".vlt");
  projectFile << "{ 'project-name " << projectName << " }";
  entryFile << "(println \"Hello, World!\")";
  projectFile.close();
  entryFile.close();
}

Obj* getConfig() { 
  auto* env = newStdEnv();
  for (const auto& file : std::filesystem::directory_iterator(".")) {
    if (file.path().extension() == ".vlt-conf") {
      auto* result = Vault::eval(env, readCode(readFile(file.path())));
      if (result->type != ValueType::DICT) {
        std::cout << "Error, config file requires a dictionary" << std::endl;
        return newUnit();
      }
      return result;
    }
  } 
  return newUnit();
}

void runProject(ArgsIter it, ArgsIter end) { 
  auto* config = getConfig();
  if (config->type == ValueType::UNIT) {
    std::cout << "Error, project is missing a config file" << std::endl;
    std::exit(0);
  }

  auto* name = Vault::get(config, newAtom("project-name"));
  if (!name || name->type != ValueType::STR) {
    std::cout << "Error, config expects to have a project-name field." << std::endl;
    std::exit(0);
  }

  std::stringstream ss;
  ss << name;
  const std::string projectName{ss.str()};
  const auto entryPath = projectName + ".vlt";

  if (!std::filesystem::exists(entryPath)) {
    std::cout << "Error, cannot find entrypoint, a file named the project name is expected to be in the root of the project" << std::endl;
    Vault::Gc::sweep();
    std::exit(0);
  }

  Vault::Gc::sweep();
  runScript(newStdEnv(), entryPath);
}

void test(const char* hello) {
  std::cout << hello << " world" << std::endl;
}

int main(const int num_args, const char* args[]) { 
  if (num_args == 1) { return repl(); }

  std::vector<std::string> pargs;
  for (int i = 0; i < num_args; i++)
    pargs.emplace_back(std::string{args[i]});

  if (pargs[1] == "new") {
    newProject(pargs.begin() + 1, pargs.end());
  } else if (pargs[1] == "run") {
    runProject(pargs.begin() + 1, pargs.end());
  } else { 
    auto* env = newStdEnv();
    runScript(env, pargs[1]); 
  } 

  return EXIT_SUCCESS;
}