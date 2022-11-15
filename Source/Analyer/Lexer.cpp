#include "Lexer.hpp"

#include "../Utils/Char.hpp"
#include "Token.hpp"

Lexer::Lexer(ErrorBuffer *errBuf, const Char8 *sourcePath, const Char8 *source)
    : errBuf(errBuf)
    , sourcePath(sourcePath)
    , source(source) {
}

Lexer::Flag Lexer::Lex(Token *out) {
  UInt8 flags;
  while (Char::IsWhitespace(this->peek)) {
    this->Advance();
  }
  if (this->peek == '\0') {
    out->kind = Token::Kind::None;
    flags |= (UInt8)Lexer::Flag::EoF;
    goto END;
  }
  if (Char::IsAlphabetic(this->peek)) {
    this->LexWord(out, &flags);
  } else if (Char::IsNumeric(this->peek)) {
    this->LexNumber(out, &flags);
  } else {
    this->LexSymbol(out, &flags);
  }
END:
  if (Lexer::HasError((Lexer::Flag)flags)) {
    out->kind = Token::Kind::None;
  }
  return (Lexer::Flag)flags;
}

Void Lexer::LexWord(Token *out, UInt8 *flags) {
  do {
    this->buffer.Put(this->peek);
    this->Advance();
  } while (Char::IsAlphabetic(this->peek) || Char::IsNumeric(this->peek) ||
           this->peek == '_');
  if (this->peek == '\0') {
    *flags |= (UInt8)Lexer::Flag::EoF;
  }
  Char8 *buffer = this->buffer.buffer;
  if (String::Compare(buffer, "procedure")) {
    out->value.Keyword.value = KeywordT::Value::Procedure;
  } else if (String::Compare(buffer, "datum")) {
    out->value.Keyword.value = KeywordT::Value::Datum;
  } else if (String::Compare(buffer, "return")) {
    out->value.Keyword.value = KeywordT::Value::Return;
  } else {
    out->kind = Token::Kind::Identifeir;
    out->value.Identifier.identifier = this->buffer.Flush();
    return;
  }
  out->kind = Token::Kind::Keyword;
}

Void Lexer::LexNumber(Token *out, UInt8 *flags) {
  out->kind = Token::Kind::Literal;
  out->value.Literal.kind = LiteralT::Kind::Integer;
  do {
    if (this->peek == '.') {
      if (out->value.Literal.kind == LiteralT::Kind::Float) {
        *flags |= (UInt8)Lexer::Flag::Error;
        this->errBuf->Put(Error(
            Error::Severity::Critical,
            "Float literal token has too many dots."
        ));
        break;
      }
      out->value.Literal.kind = LiteralT::Kind::Float;
    }
    if (this->peek != '_') {
      this->buffer.Put(this->peek);
    }
    this->Advance();
  } while (Char::IsNumeric(this->peek) || this->peek == '_' || this->peek == '.'
  );
  if (this->peek == '\0') {
    *flags |= (UInt8)Lexer::Flag::EoF;
  }
}

Void Lexer::LexSymbol(Token *out, UInt8 *flags) {
  switch (this->peek) {
  case '=':
    if (this->Peek(2) == '>') {
      out->kind = Token::Kind::Punctuator;
      out->value.Punctuator.value = PunctuatorT::Value::FArrow;
      goto DOUBLE;
    }
  case '+':
  case '-':
    out->kind = Token::Kind::Oper;
    if (this->Peek(2) == '>') {
      out->value.Oper.value = OperT::Value::RArrow;
      goto DOUBLE;
    }
    out->value.Oper.value = (OperT::Value)this->peek;
    goto SINGLE;
  case ':':
    if (this->Peek(2) == ':') {
      out->kind = Token::Kind::Oper;
      out->value.Oper.value = OperT::Value::DColon;
      goto DOUBLE;
    }
  case ';':
  case '(':
  case ')':
  case '{':
  case '}':
  case ',':
    out->kind = Token::Kind::Punctuator;
    out->value.Punctuator.value = (PunctuatorT::Value)this->peek;
    goto SINGLE;
  case '@':
  case '?':
  case '\\':
    if (this->Peek(2) == '\\') {
      do {
        this->Advance();
      } while (this->peek != '\n');
      goto SINGLE;
    }
    out->value.Modifier.value = (ModifierT::Value)this->peek;
    out->kind = Token::Kind::Modifier;
    goto SINGLE;
  default:
    *flags |= (UInt8)Lexer::Flag::Error;
    this->errBuf->Put(Error(Error::Severity::Critical, "Unkown symbolic token.")
    );
  }
DOUBLE:
  this->Advance();
SINGLE:
  this->Advance();
  if (this->peek == '\0') {
    *flags |= (UInt8)Lexer::Flag::EoF;
  }
  return;
}

Void Lexer::Advance() {
  this->peek = this->Peek();
  if (this->peek == '\n') {
    ++this->point.row;
    this->point.column = 0;
  }
  ++this->point.column;
  ++this->index;
}
