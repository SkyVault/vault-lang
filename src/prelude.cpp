#include "prelude.hpp"

using namespace Vault;

Obj* print(Obj* env, Obj* args) {
  const size_t s = Vault::len(args) ;
  if (s < 1) return newStr(""); 
  std::stringstream ss(""); 
  auto it = args;
  while (it) {
    ss << eval(env, it->asList().slot); 
    it = it->asList().next;
    if (it) ss << " ";
  } 
  const auto str = ss.str();
  std::cout << str;
  return newStr(str);
};

Obj* println(Obj* env, Obj* args) {
  auto str = print(env, args);
  std::cout << "\n";
  return str;
}

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

  push(env, newPair(newAtom("defun"), newCFun([](Obj* env, Obj* args){ 
    Obj* name = shift(args);
    Obj* params = shift(args);

    assert(params->type == ValueType::LIST);

    auto it = params;
    while (it && it->val.list.slot) { 
      assert(it->val.list.slot->type == ValueType::ATOM);
      std::cout << "-> " << it->val.list.slot << std::endl;
      it = it->val.list.next;
    }

    auto* progn = args;
    progn->type = ValueType::PROGN;

    auto* fn = newFun(env, name, params, progn);
    putInEnv(env, name, fn);
    return fn;
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

  push(env, newPair(newAtom("print"), newCFun(print))); 
  push(env, newPair(newAtom("println"), newCFun(println))); 

  return env;
}