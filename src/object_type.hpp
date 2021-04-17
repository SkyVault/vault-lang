#pragma once

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

#include "vault.hpp"

namespace Vault {

#define X_VALUE_TYPE(GEN) \
  GEN(UNIT)               \
  GEN(NUMBER)             \
  GEN(BOOL)               \
  GEN(STR)                \
  GEN(ATOM)               \
  GEN(PAIR)               \
  GEN(LIST)               \
  GEN(DICT)               \
  GEN(PROGN)              \
  GEN(FUNC)               \
  GEN(CFUNC)              \
  GEN(NATIVE_FUNC)        \
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

  enum Flags {
    NONE = 1 << 0,
    QUOTED = 1 << 1,
  };

  struct Fun {
    Obj* capturedEnv;
    Obj* name;
    Obj* params; // List of atoms
    Obj* progn;
  };

  struct FnBridge;

  union Val {
    Str atom;
    Number num;
    Str str;
    Bool boolean;
    List list; 
    CFun cfun;
    Fun fun;
    FnBridge* native;

    Dict dict;
  };

  struct Obj {
    ValueType type{ValueType::UNIT};
    Val val;

    bool mark{false};
    unsigned int flags{0};

    Obj* get(int index);
    std::string_view toStrView() const; 
    friend std::ostream& operator<<(std::ostream& os, const Obj* obj);
  };

  inline std::ostream& operator<<(std::ostream& os, const Obj* obj) {
    if (!obj) return os;
    // if (obj->mark) std::cout << "\033[1;41m";
    // else std::cout << "\033[1;46m";
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
          os << it->val.list.slot;
          it = it->val.list.next;
          if (it){ os << " "; }
        }
        os << ")";
        break;
      }

      case ValueType::DICT: { 
        std::cout << "{";
        auto it = (Obj*)obj;
        while (it) {
          auto slot = it->val.list.slot;

          auto key = slot->val.list.a;
          auto value = slot->val.list.b;

          os << "[" << key << " " << value << "] ";

          it = it->val.list.next;
          if (it){ os << " "; }
        }
        os << "}";
        break;
      }

      case ValueType::PAIR: {
        std::cout << "[" << obj->val.list.a << " " << obj->val.list.b << "]";
        break;
      }

      case ValueType::CFUNC: {
        std::cout << "<cfun>";
        break;
      }

      case ValueType::NATIVE_FUNC: {
        std::cout << "<nativefun>";
        break;
      }

      case ValueType::FUNC: {
        std::cout << "<fun:" << obj->val.fun.name << ">";
        break;
      }

      default: std::cout << "<unknown: " << ValueTypeS[obj->type] << ">"; break;
    }
    // std::cout << "\033[0m"; 
  } 
}