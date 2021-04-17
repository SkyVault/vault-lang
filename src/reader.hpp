#pragma once

#include <string_view>
#include <sstream>
#include <vector>
#include <iostream>
#include <cctype>

#include "vault.hpp"
#include "object.hpp"

#define X_TOK_TYPE(GEN) \
  GEN(TOK_END)          \
  GEN(TOK_WORD)         \
  GEN(TOK_ATOM_LIT)     \
  GEN(TOK_NUM_LIT)      \
  GEN(TOK_STR_LIT)      \
  GEN(TOK_BOOL_LIT)     \
  GEN(TOK_OPEN_PAREN)   \
  GEN(TOK_CLOSE_PAREN)  \
  GEN(TOK_OPEN_BRACE)   \
  GEN(TOK_CLOSE_BRACE)  \
  GEN(TOK_QUOTE)        \
  GEN(NUM_TOK_TYPE)

namespace Vault {
  enum TokType { X_TOK_TYPE(GEN_ENUM) };
  constexpr std::string_view TokTypeS[] = { X_TOK_TYPE(GEN_STR) };

  struct TokPosInfo {
    unsigned int line;
    unsigned int col;
  };

  struct Tok {
    TokType type{TOK_END};
    std::string lexeme{""};

    TokPosInfo info; 

    friend std::ostream& operator<<(std::ostream& os, const Tok& tok);
  };

  inline std::ostream& operator<<(std::ostream& os, const Tok& tok) {
    switch(tok.type) {
      case TokType::TOK_NUM_LIT: {
        os << "<num " << tok.lexeme << ">";
        return os;
      }
      case TokType::TOK_ATOM_LIT: {
        os << "<atom " << tok.lexeme << ">";
        return os;
      }
      case TokType::TOK_BOOL_LIT: {
        os << "<bool " << (tok.lexeme == "t" ? "true" : "false") << ">";
        return os;
      }
      case TokType::TOK_OPEN_PAREN: { os << "<(>"; return os; }
      case TokType::TOK_CLOSE_PAREN: { os << "<)>"; return os; }
      case TokType::TOK_OPEN_BRACE: { os << "<{>"; return os; }
      case TokType::TOK_CLOSE_BRACE: { os << "<}>"; return os; }
      default: {
        os << "<unknown " << tok.type << ">";
        return os;
      }
    }
  }

  Tok readToken(std::string::iterator &it, std::string::iterator &end);
  Tok peakToken(std::string::iterator &it, std::string::iterator &end);

  Obj* readCode(std::string code);
}