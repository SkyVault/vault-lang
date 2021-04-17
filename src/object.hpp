#pragma once

#include "vault.hpp"
#include "gc.hpp"

#include <cstdint>
#include <cstdlib>
#include <string_view>
#include <sstream>
#include <string>
#include <cstring>
#include <map>
#include <memory>
#include <functional>
#include <cassert>

#include "object_type.hpp"
#include "gc.hpp"

namespace Vault {

  Obj* fst(const Obj* list);
  Obj* snd(const Obj* list);

  void printObj(Obj* obj);

  Obj* newList(bool quoted=false);
  Obj* newNum(Number number=0.0);
  Obj* newBool(Bool flag=false);
  Obj* newStr(const std::string &str="");
  Obj* newAtom(const std::string &atom="", bool quoted=false);
  Obj* newProgn(bool quoted=false); 
  Obj* newDict(bool quoted=false);
  Obj* newPair(Obj* a=NULL, Obj* b=NULL, bool quoted=false);
  Obj* newCFun(CFun lambda);
  Obj* newFun(Obj* env, Obj* name, Obj* params, Obj* progn, bool quoted=false);

  Obj* newEnv();

  Obj* findInEnv(Obj* env, Obj* atom); 

  Obj* putInEnv(Obj* env, Obj* atom, Obj* value);
  Obj* putInEnvUnique(Obj* env, Obj* atom, Obj* value);
  Obj* putOrUpdateInEnv(Obj* env, Obj* atom, Obj* value);
  Obj* updateInEnv(Obj* env, Obj* atom, Obj* value);

  Obj* pushScope(Obj* env);
  Obj* popScope(Obj* env);

  static Obj* newUnit() {
    static Obj unit = {}; 
    unit.type = ValueType::UNIT;
    unit.mark = true; // Keep alive, static object shouldn't be deallocated.
    return &unit;
  }

  template <typename T> T fromObj(Obj* obj) { 
    static char buff[2048*4] = {0};
    if constexpr (std::is_same_v<T, double> || std::is_same_v<T, float> || std::is_same_v<T, int>) return (T)obj->val.num;
    if constexpr (std::is_same_v<T, bool>) return (T)obj->val.boolean;
    if constexpr (std::is_same_v<T, std::string>) {
      std::stringstream ss;
      for (int i = 0; i < obj->val.str.len; i++)
        ss << obj->val.str.data[i];
      return newStr(ss.str());
    }
    if constexpr (std::is_same_v<T, const char*>) {
      for (int i = 0; i < obj->val.str.len; i++)
        buff[i] = obj->val.str.data[i];
      buff[obj->val.str.len] = '\0';
      return buff; 
    }
  }

  template <typename T> Obj* toObj(T v) {
    if constexpr (std::is_same_v<T, double> || std::is_same_v<T, float> || std::is_same_v<T, int>) { return newNum(v); } 
    if constexpr (std::is_same_v<T, bool>) { return newBool(v); }
    if constexpr (std::is_same_v<T, const char*>) { return newStr(v); }
    if constexpr (std::is_same_v<T, std::string>) { return newStr(v); }
  } 

  struct FnBridge {
      virtual ~FnBridge() {}
      virtual Obj* operator()(const std::vector<Obj*>& params) = 0;
  };    

  // Concrete class that exposes a C++ function to the script engine.
  template <class Res, class ... Param>
  struct FnBridge_Impl : FnBridge { 
      using funcType = Res(*)(Param...);

      virtual Obj* operator()(const std::vector<Obj*>& params) {
          if (sizeof...(Param) != params.size())
              throw std::domain_error("Invalid size of parameter array");

          if constexpr (std::is_void_v<Res>) { 
            call_impl<std::tuple<Param...>>(
              func, params, std::make_index_sequence<sizeof...(Param)>()
            );
            return newUnit();
          }

          if constexpr (!std::is_void_v<Res>) {
            return toObj<Res>(
              call_impl<std::tuple<Param...>>(
                func, params, std::make_index_sequence<sizeof...(Param)>()
              )
            );
          }
      }

      template <class Tuple, std::size_t... N>
      Res call_impl(funcType func, const std::vector<Obj*>& params, std::index_sequence<N...>) {
          return func(fromObj<typename std::tuple_element<N, Tuple>::type>(params[N])...);
      }; 

      funcType func; 
      FnBridge_Impl(funcType func) : func(func) {}
  };

  template <class Res, class ... Param>
  auto wrapNativeFn(Res(*func)(Param...)) {
    return new FnBridge_Impl<Res, Param...>(func);
  }

  template <class Res, class ... Param>
  Obj* newNative(Res(*func)(Param...)) {
    Obj* obj = Vault::Gc::alloc();
    obj->type = ValueType::NATIVE_FUNC;
    obj->val.native = new FnBridge_Impl<Res, Param...>(func);
    return obj;
  }

  bool cmp(Obj* a, Obj* b);
  bool isTrue(Obj* v);

  // Lists 
  void each(Obj* list, std::function<void(Obj*)> fn); 
  size_t len(Obj* list); 
  void push(Obj* list, Obj* value); 
  Obj* cons(Obj* list, Obj* value); 
  Obj* car(Obj* list);
  Obj* cdr(Obj* list); 
  Obj* shift(Obj* &list); // Pops off the top

  // Dictionaries
  Obj* put(Obj* dict, Obj* key, Obj* value);
  Obj* get(Obj* dict, Obj* key);

  void printEnv(Obj* env);
}