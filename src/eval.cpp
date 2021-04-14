#include "eval.hpp"

using namespace Vault;

Obj* invoke(Obj* env, Obj* callable, Obj* args) {
  const auto numArgs = Vault::len(args);

  if (callable->type == ValueType::CFUNC) {
    return callable->val.cfun(env, args);
  } else if (callable->type == ValueType::FUNC) {

    auto params = callable->val.fun.params;
    auto progn = callable->val.fun.progn;

    if (len(args) != len(params)) {
      std::cout << "Error, wrong number of arguments for function: " << callable << std::endl;
      std::exit(EXIT_FAILURE);
    }

    Obj* newEnv = cons(newList(), env);

    auto it = params;
    while (it && it->val.list.slot) {
      auto v = eval(newEnv, shift(args));
      putInEnv(newEnv, it->val.list.slot, v);
      it = it->val.list.next;
    } 
    return eval(newEnv, progn);
  } else if (callable->type == ValueType::NATIVE_FUNC) {
    auto xs = std::vector<Obj*>();
    auto it = args;
    while (it) {
      xs.emplace_back(eval(env, it->val.list.slot));
      it = it->val.list.next;
    }
    return (*callable->val.native)(xs);
  } else {
    std::cout << "Can't evaluate function: " << callable << std::endl;
    return newUnit();
  }

  return newUnit();
}

Obj* evalExpr(Obj* env, Obj* obj) {
  if (!obj) return newUnit();
  if (obj->flags & Vault::Flags::QUOTED) return obj;
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
        return newUnit();
      }
      return atom;
    }

    case ValueType::LIST: {
      const auto len = Vault::len(obj);
      if (len == 0) {
        std::cout << "Error, evaluating empty list" << std::endl;
        return newUnit();
      }
      auto fn = evalExpr(env, car(obj));
      return invoke(env, fn, cdr(obj));
    }

    case ValueType::PROGN: {
      auto ret = newUnit();
      auto it = obj;
      while (it) {
        ret = evalExpr(env, it->val.list.slot);
        it = it->val.list.next;
      }
      return ret;
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