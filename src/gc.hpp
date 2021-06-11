#pragma once

#include "object_type.hpp"

#include <cstddef>
#include <cstdlib>
#include <cstdlib>
#include <iostream>

#define HEAP_CHUNK 32
#define MARK_AND_SWEAP_INTERVAL 0

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

    Obj* put(Obj* obj);

    static Obj* alloc(){ 
      auto* it = (Obj*)malloc(sizeof(Obj)); 
      return put(it); 
    }

    static Obj* root = NULL;

    void setRoot(Obj* root);

    void mark(Obj* obj);
    void sweep();

    void markAndSweep();
    void tryMarkAndSweep();

    void freeObj(Obj* obj); 
  }
}