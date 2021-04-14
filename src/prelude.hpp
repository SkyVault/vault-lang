#include "object.hpp"
#include "eval.hpp"

#include <raylib.h>
#include <cmath>

namespace Vault {
  Obj* newStdEnv();

  void initAnsiTerm(Obj* env);
  void initRaylib(Obj* env);
}