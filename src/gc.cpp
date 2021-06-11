#include "gc.hpp"

using namespace Vault::Gc;
using namespace Vault;

Obj* Vault::Gc::put(Obj* obj) { 
  if (heap.capacity == heap.size) {
    if (heap.capacity == 0) { heap.capacity = HEAP_CHUNK; }
    else heap.capacity *= 2;

    heap.buff = (Obj**)realloc(heap.buff, sizeof(Obj*)*heap.capacity);
    for (int i = heap.size; i < heap.capacity; i++) {
      heap.buff[i] = NULL;
    }
  }

  heap.buff[heap.size++] = obj;
  return obj;
}

void Vault::Gc::freeObj(Obj* obj) { 
  // std::cout << "FREEING" << std::endl;
  if (!obj) return;
  switch(obj->type) {
    case ValueType::UNIT: break;

    case ValueType::DICT: 
    case ValueType::PAIR:
    case ValueType::LIST:
    case ValueType::PROGN: 
    case ValueType::NUMBER:
    case ValueType::BOOL:
    case ValueType::CFUNC:
    case ValueType::FUNC:
      free(obj);
      break;

    case ValueType::NATIVE_FUNC: 
      delete obj->val.native;
      obj->val.native = NULL;
      free(obj);
      break;

    case ValueType::ATOM:
    case ValueType::STR:
      free(obj->val.str.data);
      free(obj);
      break;

    default:
      std::cout << "Gc doesn't know how to free object '" << obj << "'\n";
      std::exit(0);
  }
}

void Vault::Gc::mark(Obj* obj) {
  if (obj == NULL || obj->mark) return;
  obj->mark = true; 
  switch(obj->type) {
    case ValueType::UNIT:
    case ValueType::NUMBER:
    case ValueType::BOOL:
    case ValueType::ATOM:
    case ValueType::CFUNC:
    case ValueType::NATIVE_FUNC:
    case ValueType::STR: break;
    case ValueType::PAIR:
    case ValueType::PROGN:
    case ValueType::LIST:
      mark(obj->val.list.slot);
      mark(obj->val.list.next);
      break;
    case ValueType::DICT:
      mark(obj->val.dict.slot);
      mark(obj->val.dict.next);
      break;
    case ValueType::FUNC:
      mark(obj->val.fun.capturedEnv);
      mark(obj->val.fun.name);
      mark(obj->val.fun.params);
      mark(obj->val.fun.progn);
      break;
    default:
      std::cout << "Gc doesn't know how to mark object '" << obj << "'\n";
      std::exit(0);
  }
}

void sortNulls() { 
  for (int i = 0; i < heap.capacity; i++) {
    if (heap.buff[i] == NULL) { 
      for (int j = i + 1; j < heap.capacity; j++) {
        if (heap.buff[j] != NULL) {
          heap.buff[i] = heap.buff[j];
          heap.buff[j] = NULL;
          break;
        }
      }
    }
  } 
}

void Vault::Gc::sweep() {
  const auto size = heap.size;
  for (size_t i = 0; i < size; i++) {
    Obj* obj = heap.buff[i];
    if (obj && !(obj->flags & Flags::FROZEN)) {
      if (obj->mark) {
        obj->mark = false;
      } else {
        freeObj(obj);
        heap.buff[i] = NULL;
        heap.size--;
      } 
    }
  }

  sortNulls();

  // for (int i = 0; i < size; i++) {
  //   if (heap.buff[i] == NULL)
  //     std::cout << "NULL" << std::endl;
  //   else
  //     std::cout << "::" << heap.buff[i] << std::endl;
  // }
}

void Vault::Gc::setRoot(Obj* theRoot) {
  root = theRoot;
}

void Vault::Gc::tryMarkAndSweep() {
  if (!root) return;
  if (heap.tries > MARK_AND_SWEAP_INTERVAL) {
    Vault::Gc::markAndSweep();
    heap.tries = 0;
    return;
  }
  heap.tries++;
}

void Vault::Gc::markAndSweep() {
  mark(root);
  sweep();
  // std::cout << std::endl;
}