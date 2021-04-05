#pragma once

#include "object.hpp"
#include "reader.hpp"

namespace Vault {
  Obj* eval(Obj* env, Obj* val);
}