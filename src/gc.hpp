#pragma once

#include <cstddef>
#include <cstdlib>
#include <iostream>

namespace Vault {
  struct MemInfo {
    int ref{0};
  };

  template <typename T>
  T* alloc(int count = 1){
    return (T*)malloc(sizeof(T) * count); 
  }

  template <typename T>
  T* ref(T* t) {
    getMemInfo(t)->ref += 1;
    return t;
  }

  template <typename T>
  void deRef(T* t) { 
    getMemInfo(t)->ref -= 1;
    if (getMemInfo(t)->ref < 0) {
      free(getMemInfo(t));
    } 
  }

  MemInfo* getMemInfo(void* block);
}