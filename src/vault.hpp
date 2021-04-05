#pragma once

#include <string_view>
#include <iostream>
#include <sstream>
#include <string> 

#define VAULT_VERSION_MAJOR "0"
#define VAULT_VERSION_MINOR "0"
#define VAULT_VERSION_PATCH "0" 

#define VAULT_VERSION "v" VAULT_VERSION_MAJOR "." VAULT_VERSION_MINOR "." VAULT_VERSION_PATCH

#define VAULT_PROMPT "> "

#define GEN_STR(X) #X,
#define GEN_ENUM(X) X,

#define V_FALSE 0
#define V_TRUE  0

#define VAULT_VALUE_CHECK

namespace Vault {
  enum Status {
    FAILED,
    SUCCESS,
  };
}

template <typename F>
struct privDefer {
	F f;
	privDefer(F f) : f(f) {}
	~privDefer() { f(); }
};

template <typename F>
privDefer<F> defer_func(F f) {
	return privDefer<F>(f);
}

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __COUNTER__)
#define defer(code)   auto DEFER_3(_defer_) = defer_func([&](){code;})

inline std::string readInput() {
  std::string result{""}; 
  std::getline(std::cin, result); 
  return result;
}
