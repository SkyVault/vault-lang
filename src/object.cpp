#include "object.hpp"

using namespace Vault; 

Obj* newObj() {
  Obj* obj = Vault::Gc::alloc();
  obj->flags = 0;
  obj->mark = false;
  return obj;
}

Obj* Vault::newPair(Obj* a, Obj* b, bool quoted) { 
  Obj* obj = newObj();
  if (quoted) obj->flags |= Flags::QUOTED;
  obj->type = Vault::ValueType::PAIR;
  obj->val.list.slot = a;
  obj->val.list.next = b;
  return obj;
}

Obj* Vault::newList(bool quoted) {
  auto* obj = newPair(NULL, NULL);
  if (quoted) obj->flags |= Flags::QUOTED;
  obj->type = Vault::ValueType::LIST;
  return obj;
}

Obj* Vault::newNum(Number number) {
  Obj* obj = newObj();
  obj->type = Vault::ValueType::NUMBER; 
  obj->val.num = number;
  return obj;
}

Obj* Vault::newBool(Bool flag) {
  Obj* obj = newObj();
  obj->type = Vault::ValueType::BOOL; 
  obj->val.boolean = flag;
  return obj; 
}

Obj* Vault::newStr(const std::string &atom) { 
  Obj* obj = newObj();
  obj->type = Vault::ValueType::STR; 
  obj->val.atom.data = (char*)malloc(atom.length());
  obj->val.atom.len = atom.length();
  for(size_t i = 0; i < atom.length(); i++)
    obj->val.atom.data[i] = atom[i];
  return obj;
} 

Obj* Vault::newAtom(const std::string &atom, bool quoted) { 
  auto* obj = newStr(atom);
  if (quoted) obj->flags |= Flags::QUOTED;
  obj->type = Vault::ValueType::ATOM;
  return obj;
}

Obj* Vault::newProgn(bool quoted) {
  Obj* obj = newObj();
  if (quoted) obj->flags |= Flags::QUOTED;
  obj->type = Vault::ValueType::PROGN; 
  obj->val.list.next = NULL;
  obj->val.list.slot = NULL;
  return obj;
}

Obj* Vault::newCFun(CFun lambda) {
  Obj* obj = newObj();
  obj->type = ValueType::CFUNC;
  obj->val.cfun = lambda;
  return obj;
}

Obj* Vault::newFun(Obj* env, Obj* name, Obj* params, Obj* progn, bool quoted) {
  Obj* obj = newObj();
  if (quoted) obj->flags |= Flags::QUOTED;
  obj->type = ValueType::FUNC;
  obj->val.fun.capturedEnv = env;
  obj->val.fun.name = name;
  obj->val.fun.params = params;
  obj->val.fun.progn = progn; 
  return obj; 
}

Obj* Vault::newEnv(){
  auto* obj = newList(); 
  obj ->val.list.slot = newList();
  return obj;
}

Obj* findPairInScope(Obj* scope, Obj* atom) { 
  auto it = scope;
  while (it && it->val.list.slot) { 
    if (cmp(fst(it->val.list.slot), atom)) {
      return it->val.list.slot;
    }
    it = it->val.list.next;
  }
  return NULL;
}

Obj* Vault::findInEnv(Obj* env, Obj* atom){
  auto* it = env;
  while (it && it->val.list.slot) {
    auto item = it->val.list.slot;
    auto i = findPairInScope(item, atom);
    if (i) { return snd(i); }
    it = it->val.list.next;
  }
  return NULL;
}

Obj* Vault::putInEnv(Obj* env, Obj* atom, Obj* value) {
  push(env->val.list.slot, newPair(atom, value));
  return value;
}

Obj* Vault::putInEnvUnique(Obj* env, Obj* atom, Obj* value) { 
  auto* top = env->val.list.slot; 
  auto* pair = findPairInScope(top, atom);
  if (pair) { return NULL; }
  return putInEnv(env, atom, value);
}

Obj* Vault::updateInEnv(Obj* env, Obj* atom, Obj* value) {
  auto* it = env;
  while (it && it->val.list.slot) {
    auto item = it->val.list.slot;
    auto i = findPairInScope(item, atom);
    if (i) { 
      i->val.list.b = value;
      return value; 
    }
    it = it->val.list.next;
  }
  return NULL; 
}

Obj* Vault::putOrUpdateInEnv(Obj* env, Obj* atom, Obj* value){ 
  auto* top = env->val.list.slot;
  auto* pair = findPairInScope(top, atom);

  if (pair) {
    pair->val.list.b = value; 
    return value;
  } else {
    return putInEnv(env, atom, value);
  }
}

Obj* Vault::pushScope(Obj* env) {
  Obj* top = newEnv();
}

Obj* Vault::popScope(Obj* env) {

}

Obj* Vault::Obj::get(int index) { 
#ifdef VAULT_VALUE_CHECK 
  assert(this->type == ValueType::LIST || this->type == ValueType::PROGN); 
#endif 

  auto* it = this;
  while (index > 0) {
    it = it->val.list.next;
    index--;
  }

  if (index >= 0) {
    return it->val.list.slot;
  }

  return newUnit();
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
  if (v->type == ValueType::BOOL) return v->val.boolean;
  if (v->type == ValueType::NUMBER) return v->val.num == 0.0;
  return true;
}

bool Vault::cmp(Obj* a, Obj* b) {
  if (a == b) return true;
  if (a->type != b->type) return false; // TODO(Dustin): Look into auto promoting types

  switch(a->type) {
    case ValueType::NUMBER: return a->val.num == b->val.num;
    case ValueType::STR:
    case ValueType::ATOM: return
      a->val.atom.len == b->val.atom.len 
      && strncmp(a->val.atom.data, b->val.atom.data, a->val.atom.len) == 0;
    case ValueType::BOOL: return a->val.boolean == b->val.boolean;
    default: 
      assert(0);
      return false;
  }
}

size_t Vault::len(Obj* list) {
  auto it = list;
  int i = 0;
  while (it && it->val.list.slot) {
    i+=1;
    it = it->val.list.next;
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
  head->val.list.next = list;
  head->val.list.slot = value;
  return head;
}

Obj* Vault::car(Obj* list){ return list->val.list.slot; }
Obj* Vault::cdr(Obj* list){ 
  if (list->val.list.next == NULL) return newUnit();
  return list->val.list.next; 
}

Obj* Vault::fst(const Obj* list) { return list->val.list.a; } 
Obj* Vault::snd(const Obj* list) { return list->val.list.b; } 

Obj* Vault::shift(Obj* &list){
  auto* v = list->val.list.slot;
  list = list->val.list.next;
  return v;
}

void Vault::printObj(Obj* obj) {
  std::cout << "t: " << ValueTypeS[obj->type] << " v: " << obj << std::endl;
}

void Vault::printEnv(Obj* env) {
  auto scope = env;
  int scopeIndex = 0;

  while (scope && scope->val.list.slot) { 
    std::cout << "--- SCOPE " << scopeIndex << " ---\n"; 
    std::cout << scope->val.list.slot << std::endl;
    scopeIndex++;
    scope = scope->val.list.next;
  }
}