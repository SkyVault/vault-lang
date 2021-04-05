#include "object.hpp"

using namespace Vault; 

Str Obj::asAtom() {
#ifdef VAULT_VALUE_CHECK
  assert(this->type == ValueType::ATOM);
#endif
  return val.atom;
}

Number Obj::asNum() {
#ifdef VAULT_VALUE_CHECK
  assert(this->type == ValueType::NUMBER); 
#endif
  return val.num;
}

Str Obj::asStr() {
#ifdef VAULT_VALUE_CHECK 
  assert(this->type == ValueType::STR); 
#endif 
  return val.str;
}

Bool Obj::asBool() {
#ifdef VAULT_VALUE_CHECK 
  assert(this->type == ValueType::BOOL); 
#endif 
  return val.boolean;
}

List& Obj::asList() {
#ifdef VAULT_VALUE_CHECK 
  assert(this->type == ValueType::LIST || this->type == ValueType::PROGN); 
#endif 
  return val.list;
}

// MAKE

Obj* ref(Obj* obj) {
  obj->ref++;
  return obj;
}

void Vault::freeObj(Obj* obj) { 
  switch(obj->type) {
    case ValueType::ATOM: {
      free(obj->val.atom.data);
      obj->val.atom.data = nullptr;
      break;
    }
    case ValueType::STR: {
      free(obj->val.str.data);
      obj->val.str.data = nullptr;
      break;
    }
  }
  
  free(obj);
  obj = NULL;
}

void Vault::deRef(Obj *obj) {
  obj->ref -= 1;

  switch (obj->type) {
  case ValueType::LIST: {
    if (obj->val.list.next) deRef(obj->val.list.next);
    if (obj->val.list.slot) deRef(obj->val.list.slot);
    break;
  } 
  default:
    break;
  }

  if (obj->ref < 0) { 
    freeObj(obj);
  }
}

Obj* Vault::newPair(Obj* a, Obj* b) { 
  Obj* obj = Vault::alloc<Obj>();
  obj->type = Vault::ValueType::PAIR;
  obj->val.list.next = a;
  obj->val.list.slot = b;
  return obj;
}

Obj* Vault::newList() {
  auto* list = newPair(NULL, NULL);
  list->type = Vault::ValueType::LIST;
  return list;
}

Obj* Vault::newNum(Number number) {
  Obj* obj = Vault::alloc<Obj>();
  obj->type = Vault::ValueType::NUMBER; 
  obj->val.num = number;
  return obj;
}

Obj* Vault::newBool(Bool flag) {
  Obj* obj = Vault::alloc<Obj>();
  obj->type = Vault::ValueType::BOOL; 
  obj->val.boolean = flag;
  return obj; 
}

Obj* Vault::newStr(const std::string &atom) { 
  Obj* obj = Vault::alloc<Obj>();
  obj->type = Vault::ValueType::ATOM; 
  obj->val.atom.data = Vault::alloc<char>(atom.length());
  obj->val.atom.len = atom.length();
  for(size_t i = 0; i < atom.length(); i++)
    obj->val.atom.data[i] += atom[i];
  return obj;
} 

Obj* Vault::newAtom(const std::string &atom) { 
  auto* self = newStr(atom);
  self->type = Vault::ValueType::ATOM;
  return self;
}

Obj* Vault::newProgn() {
  Obj* obj = Vault::alloc<Obj>();
  obj->type = Vault::ValueType::PROGN; 
  obj->val.list.next = NULL;
  obj->val.list.slot = NULL;
  return obj;
}

Obj* Vault::Obj::get(int index) { 
#ifdef VAULT_VALUE_CHECK 
  assert(this->type == ValueType::LIST || this->type == ValueType::PROGN); 
#endif 

  auto* it = this;
  while (index > 0) {
    it = it->asList().next;
    index--;
  }

  if (index >= 0) {
    return it->asList().slot;
  }

  // TODO(Dustin): We should have our own null type
  return NULL;
}

std::string_view Vault::Obj::toStrView() const {
  #ifdef VAULT_VALUE_CHECK
    assert(this->type == ValueType::ATOM || this->type == ValueType::STR);
  #endif
  return std::string_view(this->val.atom.data, this->val.atom.len);
}

size_t Vault::len(Obj* list) {
  auto it = list;
  int i = 0;
  while (it) {
    i+=1;
    it = it->asList().next;
  }
  return i;
}

void Vault::push(Obj* list, Obj* value) {
  assert(list->type == ValueType::LIST || list->type == ValueType::PROGN);

  auto it = list; 

  if (it->val.list.next == NULL && it->val.list.slot == NULL) {
    it->val.list.slot = value;
    return;
  }

  while (it->val.list.next) {
    it = it->val.list.next;
  }

  it->val.list.next = newList();
  it->val.list.next->val.list.slot = value;
}

Obj* Vault::cons(Obj* value, Obj* list) {
  auto* head = newList();
  head->asList().next = list;
  head->asList().slot = value;
  return head;
}

Obj* Vault::car(Obj* list){
  return list->asList().slot;
}

Obj* Vault::cdr(Obj* list){
  return list->asList().next;
}
