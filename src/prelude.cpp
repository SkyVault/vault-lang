#include "prelude.hpp"

using namespace Vault;

void replace(std::string& source, const std::string& target, const std::string replacement) {
  size_t index = 0;
  while (true) {
    index = source.find(target, index);
    if (index == std::string::npos) break;
    source.replace(index, target.size(), replacement);
    index += target.size();
  }
}

std::string unescapeString(std::string str) {
  replace(str, "\\n", "\n");
  replace(str, "\\t", "\t");
  replace(str, "\\033", "\033");
  replace(str, "\\x1b", "\x1b");
  replace(str, "\\u001b", "\u001b");
  return str;
}

Obj* print(Obj* env, Obj* args) {
  const size_t s = Vault::len(args) ;
  if (s < 1) return newStr(""); 
  std::stringstream ss(""); 
  auto it = args;
  while (it) {
    ss << eval(env, it->val.list.slot); 
    it = it->val.list.next;
  } 
  const auto str = ss.str();
  std::cout << unescapeString(str);
  return newStr(str);
};

Obj* println(Obj* env, Obj* args) {
  auto str = print(env, args);
  std::cout << "\n";
  return str;
}

Obj* Vault::newStdEnv() {
  auto env = newEnv();

  putInEnv(env, newAtom("pi"), newCFun([](Obj* env, Obj* args){ 
    return newNum(3.1415926);
  })); 

  putInEnv(env, newAtom("+"), newCFun([](Obj* env, Obj* args){ 
    auto result = 0.0;
    auto it = args;
    while (it) {
      result += eval(env, it->val.list.slot)->val.num;
      it = it->val.list.next;
    }
    return newNum(result);
  })); 

  putInEnv(env, newAtom("*"), newCFun([](Obj* env, Obj* args){ 
    auto result = 1.0;
    auto it = args;
    while (it) {
      result *= eval(env, it->val.list.slot)->val.num;
      it = it->val.list.next;
    }
    return newNum(result);
  })); 

  putInEnv(env, newAtom("-"), newCFun([](Obj* env, Obj* args){ 
    auto result = 0.0;
    auto it = args;
    auto fst = true;
    while (it) {
      if (fst) { 
        result = eval(env, it->val.list.slot)->val.num;
        fst = false;
      } else result -= eval(env, it->val.list.slot)->val.num; 
      it = it->val.list.next;
    }
    return newNum(result);
  })); 

  putInEnv(env, newAtom("/"), newCFun([](Obj* env, Obj* args){ 
    auto result = 0.0;
    auto it = args;
    auto fst = true;
    while (it) {
      if (fst) { 
        result = eval(env, it->val.list.slot)->val.num;
        fst = false;
      } else result /= eval(env, it->val.list.slot)->val.num; 
      it = it->val.list.next;
    }
    return newNum(result);
  })); 

  putInEnv(env, newAtom("="), newCFun([](Obj* env, Obj* args){ 
    return newBool(cmp(eval(env, args->get(0)), eval(env, args->get(1))));
  }));

  putInEnv(env, newAtom("<"), newCFun([](Obj* env, Obj* args){ 
    return newBool(eval(env, args->get(0))->val.num < eval(env, args->get(1))->val.num);
  }));

  putInEnv(env, newAtom(">"), newCFun([](Obj* env, Obj* args){ 
    return newBool(eval(env, args->get(0))->val.num > eval(env, args->get(1))->val.num);
  }));

  putInEnv(env, newAtom("<="), newCFun([](Obj* env, Obj* args){ 
    return newBool(eval(env, args->get(0))->val.num <= eval(env, args->get(1))->val.num);
  }));

  putInEnv(env, newAtom(">="), newCFun([](Obj* env, Obj* args){ 
    return newBool(eval(env, args->get(0))->val.num >= eval(env, args->get(1))->val.num);
  }));

  putInEnv(env, newAtom("~="), newCFun([](Obj* env, Obj* args){ 
    return newBool(!cmp(eval(env, args->get(0)), eval(env, args->get(1))));
  }));

  putInEnv(env, newAtom("not"), newCFun([](Obj* env, Obj* args){ 
    return newBool(!eval(env, args->get(0))->val.boolean);
  }));

  putInEnv(env, newAtom("def"), newCFun([](Obj* env, Obj* args){ 
    auto* atom = shift(args);
    auto* value = eval(env, shift(args));

    if (!putInEnvUnique(env, atom, value)) {
      std::cout << "Error: the variable '" << atom << "' already exists in scope." << std::endl;
      std::exit(EXIT_FAILURE);
    }

    return value;
  }));

  putInEnv(env, newAtom("set"), newCFun([](Obj* env, Obj* args){
    auto v = updateInEnv(env, args->get(0), eval(env, args->get(1)));
    if (!v) {
      std::cout << "Cannot find " << args->get(0) << " in scope" << std::endl;
      std::exit(0);
    }
    return v;
  })); 

  putInEnv(env, newAtom("progn"), newCFun([](Obj* env, Obj* args){ 
    args->type = ValueType::PROGN;
    return eval(env, args);
  }));

  putInEnv(env, newAtom("do"), newCFun([](Obj* env, Obj* args){ 
    args->type = ValueType::PROGN;
    return eval(env, args);
  })); 

  putInEnv(env, newAtom("concat"), newCFun([](Obj* env, Obj* args){
    auto a = eval(env, shift(args));
    auto b = eval(env, shift(args));

    char* buff = (char*)malloc(a->val.str.len + b->val.str.len + 1);
    defer(free(buff));

    sprintf(buff, "%.*s%.*s", a->val.str.len, a->val.str.data, b->val.str.len, b->val.str.data);

    return newStr(std::string{buff}); 
  }));

  putInEnv(env, newAtom("num->int-str"), newCFun([](Obj* env, Obj* args){
    auto a = eval(env, shift(args));
    if (a->type != ValueType::NUMBER) { 
      std::cout << "Error: expected a number but got '" << a << "'" << std::endl;
      return newStr("");
    }
    return newStr(std::to_string((int)a->val.num));
  }));

  putInEnv(env, newAtom("require"), newCFun([](Obj* env, Obj* args){ 
    auto* path = eval(env, shift(args));

    char* cpath = (char*)malloc(path->val.str.len + 5);
    defer(free(cpath)); 
    sprintf(cpath, "%.*s.vlt", path->val.str.len, path->val.str.data);

    auto* progn = Vault::readCode(readFile(std::string{cpath}));
    putInEnv(env, newAtom("*source-code*"), progn);
    return Vault::eval(env, progn);
  })); 

  putInEnv(env, newAtom("defun"), newCFun([](Obj* env, Obj* args){ 
    Obj* name = shift(args); 
    Obj* params = NULL;

    if (name->type != ValueType::ATOM) {
      params = name;
      name = newAtom("anon");
    } else {
      params = shift(args);
    }

    assert(params->type == ValueType::LIST);

    auto it = params;
    while (it && it->val.list.slot) { 
      assert(it->val.list.slot->type == ValueType::ATOM);
      it = it->val.list.next;
    }

    auto* progn = args;
    progn->type = ValueType::PROGN;

    auto* fn = newFun(env, name, params, progn);
    putInEnv(env, name, fn);

    return fn;
  })); 

  putInEnv(env, newAtom("fn"), newCFun([](Obj* env, Obj* args){
    auto params = shift(args);

    auto it = params;
    while (it && it->val.list.slot) { 
      assert(it->val.list.slot->type == ValueType::ATOM);
      it = it->val.list.next;
    }

    auto* progn = args;
    progn->type = ValueType::PROGN;

    return newFun(env, newAtom("anon"), params, progn, false);
  }));

  putInEnv(env, newAtom("if"), newCFun([](Obj* env, Obj* args){
    const auto len = Vault::len(args);
    auto* comp = eval(env, args->get(0));
    if (Vault::isTrue(comp)) 
      return eval(cons(newList(), env), args->get(1));

    if (len > 2) {
      auto* f = args->get(2);
      if (f && f->type != ValueType::UNIT) 
        return eval(cons(newList(), env), f); 
    }

    return newUnit();
  })); 

  putInEnv(env, newAtom("while"), newCFun([](Obj* env, Obj* args){
    auto* expr = shift(args);
    auto* body = args;
    if (!body) { 
      while (true) {}
      return newUnit(); 
    }
    body->type = ValueType::PROGN;

    auto* newEnv = cons(newList(), env);
    while (isTrue(eval(env, expr))){
      eval(newEnv, body);
    }

    return newUnit();
  }));

  putInEnv(env, newAtom("each"), newCFun([](Obj* env, Obj* args){
    auto* xs = eval(env, shift(args));

    auto* progn = args;
    progn->type = ValueType::PROGN;

    if (xs->type != ValueType::LIST) {
      std::cout << "Error, each expects a list" << std::endl;
      std::exit(0);
    }

    auto it = xs; 
    auto* newEnv = cons(newList(), env);

    auto* value = putInEnv(newEnv, newAtom("it"), it->val.list.slot);

    while (it) { 
      *value = *it->val.list.slot; 

      if (value->type != ValueType::UNIT)
        eval(newEnv, progn);
      else 
        break;

      it = it->val.list.next;
    }

    return newUnit();
  }));

  putInEnv(env, newAtom("readln"), newCFun([](Obj* env, Obj* args){
    return newStr(readInput());
  }));


  putInEnv(env, newAtom("readln"), newCFun([](Obj* env, Obj* args){
    return newStr(readInput());
  })); 

  putInEnv(env, newAtom("print"), newCFun(print)); 
  putInEnv(env, newAtom("println"), newCFun(println)); 

  putInEnv(env, newAtom("printenv"), newCFun([](Obj* env, Obj* args){
    printEnv(env);
    return env;
  }));

  putInEnv(env, newAtom("list"), newCFun([](Obj* env, Obj* args){
    if (Vault::len(args) == 0) return newList();
    args->type = ValueType::LIST;
    return args;
  }));

  putInEnv(env, newAtom("len"), newCFun([](Obj* env, Obj* args){
    return newNum(Vault::len(eval(env, shift(args))));
  }));

  putInEnv(env, newAtom("cons"), newCFun([](Obj* env, Obj* args){
    auto* a = eval(env, shift(args));
    auto* b = eval(env, shift(args));
    return cons(a, b);
  }));

  putInEnv(env, newAtom("fst"), newCFun([](Obj* env, Obj* args){
    return car(eval(env, shift(args)));
  }));

  putInEnv(env, newAtom("snd"), newCFun([](Obj* env, Obj* args){
    return car(cdr(eval(env, shift(args))));
  }));

  putInEnv(env, newAtom("rest"), newCFun([](Obj* env, Obj* args){
    return cdr(eval(env, shift(args)));
  }));

  putInEnv(env, newAtom("rest"), newCFun([](Obj* env, Obj* args){
    return cdr(eval(env, shift(args)));
  }));

  putInEnv(env, newAtom("car"), newCFun([](Obj* env, Obj* args){
    return car(eval(env, shift(args)));
  }));

  putInEnv(env, newAtom("cdr"), newCFun([](Obj* env, Obj* args){
    return cdr(eval(env, shift(args)));
  }));

  putInEnv(env, newAtom("type?"), newCFun([](Obj* env, Obj* args){
    auto xs = eval(env, shift(args));
    switch (xs->type) {
      case ValueType::UNIT: return newAtom("unit", true);
      case ValueType::NUMBER: return newAtom("number", true);
      case ValueType::STR: return newAtom("str", true);
      case ValueType::ATOM: return newAtom("atom", true);
      case ValueType::BOOL: return newAtom("bool", true);
      case ValueType::PAIR: return newAtom("pair", true);
      case ValueType::LIST: return newAtom("list", true);
      case ValueType::DICT: return newAtom("dict", true);
      case ValueType::PROGN: return newAtom("progn", true);
      case ValueType::FUNC: return newAtom("func", true);
      case ValueType::CFUNC: return newAtom("cfunc", true);
      case ValueType::NATIVE_FUNC: return newAtom("native-func", true);
      default: {
        assert(0);
      }
    }
  }));

  putInEnv(env, newAtom("empty?"), newCFun([](Obj* env, Obj* args){
    auto xs = eval(env, shift(args));
    return newBool(len(xs) == 0);
  })); 

  putInEnv(env, newAtom("let"), newCFun([](Obj* env, Obj* args){
    auto* bindings = shift(args);
    assert(bindings->type == ValueType::LIST);
    args->type = ValueType::PROGN; 
    auto* newEnv = cons(newList(), env);
    auto* it = bindings;
    while (it) {
      auto* atom = it->val.list.slot; 
      assert(atom->type == ValueType::ATOM);
      it = it->val.list.next;
      auto* val = eval(newEnv, it->val.list.slot); 
      putInEnv(newEnv, atom, val);
      it = it->val.list.next;
    } 
    return eval(newEnv, args); 
  })); 

  putInEnv(env, newAtom("init-ansi-term-env"), newCFun([](Obj* env, Obj* args){
    initAnsiTerm(env);
    return newUnit();
  }));

  putInEnv(env, newAtom("init-raylib"), newCFun([](Obj* env, Obj* args){
    initRaylib(env);
    return newUnit();
  }));

  putInEnv(env, newAtom("sin"), newNative(sin));
  putInEnv(env, newAtom("cos"), newNative(cos));
  putInEnv(env, newAtom("atan2"), newNative(atan2));

  return env;
}

std::string readChar(int i) {
  char chr = '\0';

  // Redirect cout.
  std::streambuf* oldCoutStreamBuf = std::cout.rdbuf();
  std::ostringstream strCout;
  std::cout.rdbuf( strCout.rdbuf() );

  system("stty raw");
  chr = getchar();
  system("stty cooked");

  std::cout.rdbuf( oldCoutStreamBuf ); 

  return std::string{chr};
}

void Vault::initAnsiTerm(Obj* env) {
  putInEnv(env, newAtom("term/read-char"), newNative(readChar)); 

  putInEnv(env, newAtom("term/get-size"), newCFun([](Obj* env, Obj* args){ 
#ifdef __unix__
    struct winsize sz;
    static int inout; 
    memset(&sz, 0, sizeof(sz));
    ioctl(inout, TIOCGWINSZ, &sz);

    Obj* res = newList();
    push(res, newNum(sz.ws_row));
    push(res, newNum(sz.ws_col));
    return res;
#else 
    Obj* res = newList();
    push(res, newNum(0));
    push(res, newNum(0));
    return res;
#endif
  })); 
} 

void drawRect(double x, double y, double w, double h) {
  DrawRectangle((int)x, (int)y, (int)w, (int)h, RED);
}

void clearBg() {
  ClearBackground(BLACK);
}

void Vault::initRaylib(Obj* env) {
  putInEnv(env, newAtom("rl-init-window"), newNative(InitWindow));
  putInEnv(env, newAtom("rl-window-should-close"), newNative(WindowShouldClose));
  putInEnv(env, newAtom("rl-begin-drawing"), newNative(BeginDrawing));
  putInEnv(env, newAtom("rl-end-drawing"), newNative(EndDrawing));
  putInEnv(env, newAtom("rl-draw-rectangle"), newNative(drawRect));
  putInEnv(env, newAtom("rl-frame-time"), newNative(GetFrameTime));
  putInEnv(env, newAtom("rl-clear-background"), newNative(clearBg));
  putInEnv(env, newAtom("rl-draw-fps"), newNative(DrawFPS));
  // putInEnv(env, newAtom("rl-is-key-pressed"), newNative(IsKeyPressed));
  // putInEnv(env, newAtom("rl-is-key-down"), newNative(IsKeyDown));
}