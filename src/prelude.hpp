#include "object.hpp"
#include "eval.hpp"

#include <raylib.h>
#include <cmath>
#include <ctime>

#ifdef __unix__
#include <termios.h>
#include <sys/ioctl.h> 
#endif

namespace Vault {
  Obj* newStdEnv();

  void initAnsiTerm(Obj* env);
}