#include "reader.hpp"

using namespace Vault;

std::string fromRange(std::string::iterator start, std::string::iterator end) { 
    std::stringstream ss;
    while (start < end) { 
      ss << *start;
      start++;
    }
    return ss.str();
}

bool IsDelim(char ch) {
  return std::isspace(ch) || ch == '(' || ch == ')';
}

Tok readStr(std::string::iterator& it, std::string::iterator& end) { 
  it++;
  auto start = it;
  while (it != end && *it != '\"') {
    it++;
  }
  const auto stop = it;
  it += 1;
  auto str = fromRange(start, stop);
  return Tok{TokType::TOK_STR_LIT, str};
}

Tok Vault::readToken(std::string::iterator& it, std::string::iterator& end) {
  while (it != end && std::isspace(*it)) { it += 1; }
  if (it >= end) return Tok{};

  bool isNeg = false;
  bool isDec = false;

  // Skip comments
  if (*it == ';') {
    while (it != end && *it != '\n') it++;
    while (it != end && std::isspace(*it)) { it += 1; }
    if (it == end) return Tok{};
  }

  auto start = it;

  // Parsing numbers
  if (*it == '-') { isNeg = true; it++; }
  if (*it == '.') { isDec = true; it++; }

  if (std::isdigit(*it)) {
    while (it != end) {
      if (*it == '.') {
        if (isDec) break;
        else isDec = true;
        it++;
        continue;
      }

      if (!std::isdigit(*it)) break; 

      it++;
    }

    return Tok{
      .type = TokType::TOK_NUM_LIT,
      .lexeme = fromRange(start, it),
    };
  } 

  // True and false literal
  if (*it == '#' && *(it + 1) == 'f') {
    it += 2; 
    return Tok{ TokType::TOK_BOOL_LIT, "f" };
  }

  if (*it == '#' && *(it + 1) == 't') {
    it += 2; 
    return Tok{ TokType::TOK_BOOL_LIT, "t" };
  }

  // parsing single character lexemes
  if (*it == '(') {
    it += 1;
    return Tok{ TokType::TOK_OPEN_PAREN, "(" }; 
  }

  if (*it == ')') {
    it += 1;
    return Tok{ TokType::TOK_CLOSE_PAREN, ")" }; 
  }

  if (*it == '\'') {
    it += 1;
    return Tok{TokType::TOK_QUOTE, "'"};
  }

  if (*it == '\"') {
    return readStr(it, end);
  }

  while (it != end  && !IsDelim(*it)) {
    it++;
  }

  return Tok{TokType::TOK_ATOM_LIT, fromRange(start, it)};
}

Obj* readExpr(std::string::iterator& it, std::string::iterator& end) {
  auto tok = readToken(it, end);

  bool quoted = false;
  if (tok.type == TokType::TOK_QUOTE) {
    quoted = true;
    tok = readToken(it, end);
  }

  switch(tok.type){
    case TokType::TOK_NUM_LIT: return newNum(std::stod(tok.lexeme));
    case TokType::TOK_ATOM_LIT: return newAtom(tok.lexeme, quoted);
    case TokType::TOK_BOOL_LIT: return newBool(tok.lexeme == "t");
    case TokType::TOK_STR_LIT: return newStr(tok.lexeme);

    case TokType::TOK_OPEN_PAREN: {
      auto* list = newList(quoted);

      while (it != end) {
        auto* item = readExpr(it, end);
        if (item->type == ValueType::UNIT) {
          return list;
        }
        Vault::push(list, item);
      }

      std::cout << "Unbalanced parenthises" << std::endl;
      return newUnit();
    }

    case TokType::TOK_END:
    case TokType::TOK_CLOSE_PAREN: { return newUnit(); }

    default: {
      std::cout << "Unhandled token in readExpr: " << TokTypeS[tok.type] << " = " << tok.lexeme <<  std::endl;
      std::exit(0);
      break;
    }
  }
}

Obj* readProgn(std::string::iterator& it, std::string::iterator& end) {
  Obj* progn = newProgn(); 
  while(it < end) {
    auto expr = readExpr(it, end); 
    Vault::push(progn, expr);
  }
  return progn;
}

Obj* Vault::readCode(std::string code) {
  std::string::iterator it = code.begin();
  std::string::iterator end = code.end();
  return readProgn(it, end);
}