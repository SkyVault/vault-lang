#include "eval.hpp"

using namespace Vault;

Obj* invoke(Obj* env, Obj* callable, Obj* args) {
  const auto numArgs = Vault::len(args);

  if (callable->type == ValueType::CFUNC) {
    return callable->val.cfun(env, args);
  } else {
    std::cout << "Can't evaluate function: " << callable << std::endl;
    return newUnit();
  }

  return newUnit();
}

Obj* evalExpr(Obj* env, Obj* obj) {
  switch(obj->type) {
    case ValueType::UNIT:
    case ValueType::BOOL:
    case ValueType::NUMBER:
    case ValueType::STR:
      return obj;

    case ValueType::ATOM: {
      auto* atom = findInEnv(env, obj);
      if (!atom || atom->type == ValueType::UNIT) {
        std::cout << "Cannot find '" << obj << "' in the environment" << std::endl;
      }
      return atom;
    }

    case ValueType::LIST: {
      const auto len = Vault::len(obj);
      if (len == 0) {
        std::cout << "Error, evaluating empty list" << std::endl;
        return newUnit();
      }
      return invoke(env, evalExpr(env, car(obj)), cdr(obj));
    }

    case ValueType::PROGN: {
      auto it = obj;
      while (it) {
        it->asList().slot = evalExpr(env, it->asList().slot);
        it = it->asList().next;
      }
      return obj;
    }
    default: {
      std::cout << "Unhandled expr type in evalExpr: " << ValueTypeS[obj->type] << std::endl;
      std::cout << "> " << obj << std::endl;
      std::exit(0);
    }
  }
  return obj;
}

Obj* Vault::eval(Obj* env, Obj* obj) {
  return evalExpr(env, obj);
}