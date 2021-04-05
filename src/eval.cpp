#include "eval.hpp"

using namespace Vault;

Obj* invoke(Obj* callable, Obj* args) {
  const auto numArgs = Vault::len(args);

  if (callable->type == ValueType::ATOM) {
    if (callable->toStrView() == "+") {
      double result = 0.0;
      for (int i = 0; i < numArgs; i++) {
        result += args->get(i)->asNum();
      }
      return newNum(result);
    }
  }

  return newUnit();
}

Obj* evalExpr(Obj* env, Obj* obj) {
  switch(obj->type) {
    case ValueType::ATOM:
    case ValueType::BOOL:
    case ValueType::NUMBER:
    case ValueType::STR:
      return obj;

    case ValueType::LIST: {
      const auto len = Vault::len(obj);
      if (len == 0) {
        std::cout << "Error, evaluating empty list" << std::endl;
        return newUnit();
      }
      return invoke(evalExpr(env, car(obj)), cdr(obj));
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