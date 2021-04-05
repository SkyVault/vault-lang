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
  assert(
    this->type == ValueType::LIST 
    || this->type == ValueType::PROGN 
    || this->type == ValueType::PAIR
  ); 
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
  case ValueType::UNIT: break;
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
  obj->val.list.slot = a;
  obj->val.list.next = b;
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

Obj* Vault::newCFun(CFun lambda) {
  Obj* obj = Vault::alloc<Obj>();
  obj->type = ValueType::CFUNC;
  obj->val.cfun = lambda;
  return obj;
}

Obj* Vault::newEnv(){
  auto list = newList(); 
  return list;
}

Obj* findPairInEnv(Obj* env, Obj* atom) { 
  const size_t envSize = Vault::len(env);
  for(int i = envSize - 1; i >= 0; i--) {
    auto* pair = env->get(i);
    if (cmp(fst(pair), atom)) {
      return pair;
    }
  }
  return NULL;
}

Obj* Vault::findInEnv(Obj* env, Obj* atom){
  auto p = findPairInEnv(env, atom);
  if (!p) return newUnit();
  return snd(p);
}

Obj* Vault::putInEnv(Obj* env, Obj* atom, Obj* value) {
  auto exists = findPairInEnv(env, atom);
  if (exists) { exists->val.list.b = value; return value; }
  push(env, newPair(atom, value));
  return value;
}

Obj* Vault::pushEnv(Obj* env){

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

bool Vault::isTrue(Obj* v) {
  if (!v) return false;
  if (v->type == ValueType::UNIT) return false;
  if (v->type == ValueType::BOOL) return v->asBool();
  if (v->type == ValueType::NUMBER) return v->asNum() == 0.0;
  return true;
}

bool Vault::cmp(Obj* a, Obj* b) {
  if (a == b) return true;
  if (a->type != b->type) return false; // TODO(Dustin): Look into auto promoting types

  switch(a->type) {
    case ValueType::NUMBER: return a->asNum() == b->asNum();
    case ValueType::STR:
    case ValueType::ATOM: return
      a->asAtom().len == b->asAtom().len 
      && strncmp(a->asAtom().data, b->asAtom().data, a->asAtom().len) == 0;
    case ValueType::BOOL: return a->asBool() == b->asBool();
    default: 
      assert(0);
      return false;
  }
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

void Vault::each(Obj* list, std::function<void(Obj*)> fn) {
#ifdef VAULT_VALUE_CHECK
  assert(list->type == ValueType::LIST || list->type == ValueType::PAIR);
#endif

  auto it = list;
  while (it) {
    fn(it->val.list.slot);
    it = it->val.list.next;
  }
}

Obj* Vault::cons(Obj* value, Obj* list) {
  auto* head = newList();
  head->asList().next = list;
  head->asList().slot = value;
  return head;
}

Obj* Vault::car(Obj* list){ return list->val.list.slot; }
Obj* Vault::cdr(Obj* list){ return list->val.list.next; }

Obj* Vault::fst(Obj* list) { return list->val.list.a; } 
Obj* Vault::snd(Obj* list) { return list->val.list.b; } 