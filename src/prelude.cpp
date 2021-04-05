#include "prelude.hpp"

using namespace Vault;

Obj* Vault::newStdEnv() {
  auto env = newEnv();

  push(env, newPair(newAtom("pi"), newCFun([](Obj* env, Obj* args){ 
    return newNum(3.1415926);
  }))); 

  push(env, newPair(newAtom("+"), newCFun([](Obj* env, Obj* args){ 
    auto result = 0.0;
    auto it = args;
    while (it) {
      result += eval(env, it->asList().slot)->asNum();
      it = it->asList().next;
    }
    return newNum(result);
  }))); 

  push(env, newPair(newAtom("*"), newCFun([](Obj* env, Obj* args){ 
    auto result = 1.0;
    auto it = args;
    while (it) {
      result *= eval(env, it->asList().slot)->asNum();
      it = it->asList().next;
    }
    return newNum(result);
  }))); 

  push(env, newPair(newAtom("-"), newCFun([](Obj* env, Obj* args){ 
    auto result = 0.0;
    auto it = args;
    auto fst = true;
    while (it) {
      if (fst) { 
        result = eval(env, it->asList().slot)->asNum();
        fst = false;
      } else result -= eval(env, it->asList().slot)->asNum(); 
      it = it->asList().next;
    }
    return newNum(result);
  }))); 

  push(env, newPair(newAtom("/"), newCFun([](Obj* env, Obj* args){ 
    auto result = 0.0;
    auto it = args;
    auto fst = true;
    while (it) {
      if (fst) { 
        result = eval(env, it->asList().slot)->asNum();
        fst = false;
      } else result /= eval(env, it->asList().slot)->asNum(); 
      it = it->asList().next;
    }
    return newNum(result);
  }))); 

  push(env, newPair(newAtom("eq"), newCFun([](Obj* env, Obj* args){ 
    return newBool(cmp(eval(env, args->get(0)), eval(env, args->get(1))));
  })));

  push(env, newPair(newAtom("neq"), newCFun([](Obj* env, Obj* args){ 
    return newBool(!cmp(eval(env, args->get(0)), eval(env, args->get(1))));
  })));

  push(env, newPair(newAtom("not"), newCFun([](Obj* env, Obj* args){ 
    return newBool(!eval(env, args->get(0))->asBool());
  })));

  push(env, newPair(newAtom("set"), newCFun([](Obj* env, Obj* args){
    return putInEnv(env, args->get(0), eval(env, args->get(1)));
  }))); 

  push(env, newPair(newAtom("if"), newCFun([](Obj* env, Obj* args){
    auto* comp = eval(env, args->get(0));
    auto* f = args->get(2);
    if (Vault::isTrue(comp)) 
      return eval(env, args->get(1));
    else if (f && f->type != ValueType::UNIT) 
      return eval(env, f); 
  }))); 

  push(env, newPair(newAtom("readln"), newCFun([](Obj* env, Obj* args){
    return newStr(readInput());
  })));

  return env;
}