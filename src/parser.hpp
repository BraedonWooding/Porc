#ifndef PARSER_HPP
#define PARSER_HPP

#include "token_stream.hpp"
#include "ast.hpp"
#include "err_stream.hpp"

#include <string>

namespace porc::internals {

template<typename T>
using optional_unique_ptr  = std::optional<std::unique_ptr<T>>;

class Parser {
private:
  TokenStream stream;
  ErrStream err;

  void UnexpectedToken(Token actual, Token expected);
  void UnexpectedEndOfFile(Token::Kind expected);
  void UnexpectedToken(Token actual, std::string msg);
  bool ConsumeToken(Token::Kind type);

  std::optional<std::vector<std::string>>
    ParseIdentifierAccess(Token::Kind continuer);

  std::optional<std::variant<std::vector<VarDecl::Declaration>,
           std::vector<std::unique_ptr<Expr>>>>
    ParseAssignmentIdentifierList(Token::Kind continuer);

  std::unique_ptr<Expr> ConvIdentToExpr(std::string id, LineRange pos);
  void ConvListDeclsToExpr(std::vector<TupleDecl::ArgDecl> &decls,
                           std::vector<std::unique_ptr<Expr>> &exprs);
  std::unique_ptr<Expr> ParenthesiseExpr(std::unique_ptr<Expr> expr);

  optional_unique_ptr<Expr> ParseExprFuncOrStruct(std::unique_ptr<TupleDecl> decl);

  /*

    These could be improved, I haven't yet been able to find a method to do this
    my belief is that I can use fold expressions to partially eval all them
    But I don't know if it is possible.

  */

  template<typename Fn, typename ForEach>
  bool ParseListConjugate(Fn fn, ForEach for_each,
                                      Token pair_sep = Token::Colon,
                                      Token next_pair = Token::Comma) {
    int index = 0;
    while (true) {
      auto first = (this->*fn)();
      if (!first) return first.error();
      if (!ConsumeToken(pair_sep.type))
        return ParseError(ParseError::Kind::MissingToken, "Requiring separator",
                          pair_sep);
      auto second = (this->*fn)();
      if (!second) return second.error();
      for_each(index, std::move(*first), std::move(*second));
      if (stream.PeekCur().type == next_pair.type) {
        index++;
        stream.PopCur();
      } else {
        break;
      }
    }
    return true;
  }

  template<typename Fn, typename ForEach>
  bool ParseList(Fn fn, ForEach for_each,
                                      Token continuer = Token::Comma) {
    int index = 0;
    while (true) {
      auto res = (this->*fn)();
      if (!res) return res.error();
      for_each(index, std::move(*res));
      if (stream.PeekCur().type == continuer.type) {
        index++;
        stream.PopCur();
      } else {
        break;
      }
    }
    return true;
  }

  template<typename To>
  std::optional<To> TryTokenCast(Token tok);

  std::optional<TupleDecl::ArgDecl> ParseTupleDeclSegment();
  optional_unique_ptr<Expr> ParseExprOrTupleDecl();
  optional_unique_ptr<TupleDecl> ParseRestTupleDeclExpr(
    std::vector<TupleDecl::ArgDecl> declarations);
  std::optional<IfBlock::IfStatement> ParseIfStatement();

public:
  Parser(TokenStream stream, std::ostream &out = std::cerr)
      : stream(std::move(stream)), err(out) {}

  optional_unique_ptr<VarDecl> ParseFileFuncDecl();
  optional_unique_ptr<VarDecl> ParseFileStructDecl();

  optional_unique_ptr<FileDecl> ParseFileDecl();
  optional_unique_ptr<TupleDecl> ParseTupleDecl();
  optional_unique_ptr<VarDecl> ParseVarDecl();
  optional_unique_ptr<VarDecl>
    ParseVarDeclWithDeclList(std::vector<VarDecl::Declaration> lhs);

  optional_unique_ptr<FileBlock> ParseFileBlock();
  optional_unique_ptr<FuncBlock> ParseFuncBlock();
  optional_unique_ptr<IfBlock> ParseIfBlock();
  optional_unique_ptr<WhileBlock> ParseWhileBlock();
  optional_unique_ptr<ForBlock> ParseForBlock();

  optional_unique_ptr<FuncCall> ParseFuncCall();
  optional_unique_ptr<Constant> ParseConstant();

  optional_unique_ptr<MacroExpr> ParseMacroExpr();
  optional_unique_ptr<TypeExpr> ParseTypeExpr();
  optional_unique_ptr<AssignmentExpr> ParseAssignmentExpr();
  optional_unique_ptr<Expr> ParseExpr();
  optional_unique_ptr<Expr::MapExpr> ParseMapExpr();
  optional_unique_ptr<Expr::CollectionExpr> ParseArrayExpr();
  optional_unique_ptr<Expr::CollectionExpr> ParseTupleExpr();
  optional_unique_ptr<PostfixExpr> ParsePostfixExpr();
  optional_unique_ptr<UnaryExpr> ParseUnaryExpr();
  optional_unique_ptr<PowerExpr> ParsePrefixExpr();
  optional_unique_ptr<MultiplicativeExpr> ParseMultiplicativeExpr();
  optional_unique_ptr<AdditiveExpr> ParseAdditiveExpr();
  optional_unique_ptr<RelationalExpr> ParseRelationalExpr();
  optional_unique_ptr<EqualityExpr> ParseEqualityExpr();
  optional_unique_ptr<LogicalAndExpr> ParseLogicalAndExpr();
  optional_unique_ptr<LogicalOrExpr> ParseLogicalOrExpr();
};

}

#endif