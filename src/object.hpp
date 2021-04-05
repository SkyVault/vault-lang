#pragma once

#include "vault.hpp"
#include "gc.hpp"

#include <cstdint>
#include <cstdlib>
#include <string_view>
#include <string>
#include <cstring>
#include <map>
#include <memory>
#include <functional>
#include <cassert>

namespace Vault {

#define X_VALUE_TYPE(GEN) \
  GEN(UNIT)               \
  GEN(NUMBER)             \
  GEN(BOOL)               \
  GEN(ATOM)               \
  GEN(STR)                \
  GEN(PAIR)               \
  GEN(LIST)               \
  GEN(DICT)               \
  GEN(PROGN)              \
  GEN(FUNC)               \
  GEN(CFUNC)              \
  GEN(NUM_VALUE_TYPE) 

  enum ValueType { X_VALUE_TYPE(GEN_ENUM) }; 
  constexpr std::string_view ValueTypeS[] = { X_VALUE_TYPE(GEN_STR) };

#undef X_VALUE_TYPE

  struct Str {
    char* data{nullptr};
    size_t len{0};
  }; 

  struct Obj;

  struct Pair {
    union {
      struct { Obj* slot; Obj* next; };
      struct { Obj* a; Obj* b; };
    };
  };

  using List = Pair; 
  using Dict = List; // List of pairs (k, v), in the future this will be an actual hashmap

  using Env = Dict;

  using Bool = uint8_t;
  using Number = double;

  typedef Obj*(*CFun)(Obj*, Obj*);

  union Val {
    Str atom;
    Number num;
    Str str;
    Bool boolean;
    List list; 
    Dict dict;
    CFun cfun;
  };

  struct Obj {
    ValueType type{ValueType::UNIT};
    Val val;

    int ref{0};

    Str asAtom();
    Number asNum();
    Str asStr();
    Bool asBool();
    List& asList();
    Dict& asDict();

    Obj* get(int index);
    std::string_view toStrView() const;

    friend std::ostream& operator<<(std::ostream& os, const Obj* obj);
  };

  inline std::ostream& operator<<(std::ostream& os, const Obj* obj) {
    switch(obj->type) {
      case ValueType::UNIT: { os << "()"; break; }
      case ValueType::ATOM: { os << obj->toStrView(); break; }
      case ValueType::NUMBER: { os << obj->val.num; break; }
      case ValueType::STR: { os << obj->toStrView(); break; }
      case ValueType::BOOL: { os << (obj->val.boolean ? "#t" : "#f"); break; }

      case ValueType::LIST: 
      case ValueType::PROGN: { 
        std::cout << "(";
        auto it = (Obj*)obj;
        while (it) {
          os << it->asList().slot;
          it = it->asList().next;
          if (it){ os << " "; }
        }
        os << ")";
        break;
      }

      case ValueType::CFUNC: {
        std::cout << "<cfun>";
        break;
      }

      default: std::cout << "<unknown>"; break;
    }
  }

  Obj* newList();
  Obj* newNum(Number number=0.0);
  Obj* newBool(Bool flag=false);
  Obj* newStr(const std::string &str="");
  Obj* newAtom(const std::string &atom="");
  Obj* newProgn(); 
  Obj* newPair(Obj* a=NULL, Obj* b=NULL);
  Obj* newCFun(CFun lambda);

  Obj* newEnv();

  Obj* findInEnv(Obj* env, Obj* atom);
  Obj* putInEnv(Obj* env, Obj* atom, Obj* value);
  Obj* pushEnv(Obj* env);

  static Obj* newUnit() {
    static Obj unit = {}; 
    unit.type = ValueType::UNIT;
    return &unit;
  }

  bool cmp(Obj* a, Obj* b);
  bool isTrue(Obj* v);

  void each(Obj* list, std::function<void(Obj*)> fn);

  size_t len(Obj* list);

  void push(Obj* list, Obj* value);

  Obj* cons(Obj* list, Obj* value); 
  Obj* car(Obj* list);
  Obj* cdr(Obj* list);

  Obj* fst(Obj* list);
  Obj* snd(Obj* list);

  void freeObj(Obj* obj); 
  Obj* ref(Obj* obj);
  void deRef(Obj* obj);
}