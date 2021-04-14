#include "object.hpp"
#include "eval.hpp"

#include <raylib.h>
#include <cmath>

#ifdef __unix__
#include <termios.h>
#endif

namespace Vault {
  Obj* newStdEnv();

  void initAnsiTerm(Obj* env);
  void initRaylib(Obj* env);
}