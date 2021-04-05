#pragma once

#include "vault.hpp"
#include "gc.hpp"

#include <cstdint>
#include <cstdlib>
#include <string_view>
#include <map>
#include <memory>
#include <cassert>

#define X_VALUE_TYPE(GEN) \
  GEN(UNIT)               \
  GEN(NUMBER)             \
  GEN(BOOL)               \
  GEN(ATOM)               \
  GEN(STR)                \
  GEN(PAIR)               \
  GEN(LIST)               \
  GEN(DICT)               \
  GEN(FUNC)               \
  GEN(PROGN)              \
  GEN(NUM_VALUE_TYPE)

namespace Vault {
  enum ValueType {
    X_VALUE_TYPE(GEN_ENUM)
  };

  constexpr std::string_view VaultTypeS[] = { X_VALUE_TYPE(GEN_STR) };

  struct Str {
    char* data{nullptr};
    size_t len{0};
  }; 

  struct Obj;

  struct Pair {
    union {
      struct { Obj* next; Obj* slot; };
      struct { Obj* a; Obj* b; };
    };
  };

  using List = Pair; 
  using Dict = List; // List of pairs (k, v), in the future this will be an actual hashmap

  using Env = Dict;

  using Bool = uint8_t;
  using Number = double;

  union Val {
    Str atom;
    Number num;
    Str str;
    Bool boolean;
    List list; 
    Dict dict;
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

  static Obj* newUnit() {
    static Obj unit = {}; 
    unit.type = ValueType::UNIT;
    return &unit;
  }

  size_t len(Obj* list);

  void push(Obj* list, Obj* value);

  Obj* cons(Obj* list, Obj* value); 
  Obj* car(Obj* list);
  Obj* cdr(Obj* list);

  void freeObj(Obj* obj); 
  Obj* ref(Obj* obj);
  void deRef(Obj* obj);
}