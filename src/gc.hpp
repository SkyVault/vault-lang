#pragma once

#include "object_type.hpp"

#include <cstddef>
#include <cstdlib>
#include <cstdlib>
#include <iostream>

#define HEAP_CHUNK 32
#define MARK_AND_SWEAP_INTERVAL 100

namespace Vault { 
  namespace Gc { 

    static struct { 
      Obj** buff;
      size_t capacity;
      size_t size;

      int tries;
    } heap = {
      .buff = NULL,
      .capacity = 0,
      .size = 0,
      .tries = 0,
    };

    void put(Obj* obj);

    static Obj* alloc(){ 
      auto* it = (Obj*)malloc(sizeof(Obj)); 
      put(it); 
      return it;
    }

    void mark(Obj* obj);
    void sweep();

    void markAndSweep(Obj* root);
    void tryMarkAndSweep(Obj* root);

    void freeObj(Obj* obj); 
  }
}