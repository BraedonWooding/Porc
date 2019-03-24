#ifndef PARSER_HPP
#define PARSER_HPP

#include "token_stream.hpp"
#include "ast.hpp"

#include <expected.hpp>
#include <string>

namespace porc::internals {

template<typename T, typename E>
using expected_unique_ptr = tl::expected<std::unique_ptr<T>, E>;

template<typename T>
using optional_unique_ptr = std::optional<std::unique_ptr<T>>;

struct ParseError {
  enum class Kind {
    MissingToken, // missing a token (not EOF)
    InvalidToken, // wasn't expecting this token
    ValidEOF,     // not really an error more just info that we ran out
    InvalidEOF,   // wasn't expecting EOF
  };

public:
  Kind kind;
  std::string extra_info;
  Token related_token;

  ParseError(Kind kind) : kind(kind) {}
  ParseError(Kind kind, std::string extra_info)
      : kind(kind), extra_info(extra_info) {}
  ParseError(Kind kind, std::string extra_info, Token related_token)
      : kind(kind), extra_info(extra_info), related_token(related_token) {}
};

class Parser {
  template<typename T>
  using expected_expr = expected_unique_ptr<T, ParseError>;

private:
  TokenStream stream;

  void UnexpectedToken(Token actual, Token expected);
  void UnexpectedEndOfFile(Token::Kind expected);
  void UnexpectedToken(Token actual, std::string msg);
  bool ConsumeToken(Token::Kind type);

  tl::expected<std::vector<std::string>, ParseError>
    ParseIdentifierAccess(Token::Kind continuer);

  tl::expected<std::variant<std::vector<VarDecl::Declaration>,
           std::vector<std::unique_ptr<Expr>>>, ParseError>
    ParseAssignmentIdentifierList(Token::Kind continuer);

  std::unique_ptr<Expr> ConvIdentToExpr(std::string id, LineRange pos);
  void ConvListDeclsToExpr(std::vector<TupleDecl::ArgDecl> &decls,
                           std::vector<std::unique_ptr<Expr>> &exprs);
  std::unique_ptr<Expr> ParenthesiseExpr(std::unique_ptr<Expr> expr);

  expected_expr<Expr> ParseExprFuncOrStruct(std::unique_ptr<TupleDecl> decl);

  template<typename Fn, typename ForEach>
  std::optional<ParseError> ParseListConjugate(Fn fn, ForEach for_each,
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
    return std::nullopt;
  }

  template<typename Fn, typename ForEach>
  std::optional<ParseError> ParseList(Fn fn, ForEach for_each,
                                      Token continuer = Token::Comma) {
    int index = 0;
    // auto(Parser::*func_pointer) = fn;
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
    return std::nullopt;
  }

  tl::expected<TupleDecl::ArgDecl, ParseError> ParseTupleDeclSegment();
  expected_expr<Expr> ParseExprOrTupleDecl();
  expected_expr<TupleDecl> ParseRestTupleDeclExpr(
    std::vector<TupleDecl::ArgDecl> declarations);
  tl::expected<IfBlock::IfStatement, ParseError> ParseIfStatement();

public:
  Parser(TokenStream stream) : stream(std::move(stream)) {}

  expected_expr<VarDecl> ParseFileFuncDecl();
  expected_expr<VarDecl> ParseFileStructDecl();

  expected_expr<FileDecl> ParseFileDecl();
  expected_expr<TupleDecl> ParseTupleDecl();
  expected_expr<VarDecl> ParseVarDecl();
  expected_expr<VarDecl>
    ParseVarDeclWithDeclList(std::vector<VarDecl::Declaration> lhs);

  expected_expr<FileBlock> ParseFileBlock();
  expected_expr<FuncBlock> ParseFuncBlock();
  expected_expr<IfBlock> ParseIfBlock();
  expected_expr<WhileBlock> ParseWhileBlock();
  expected_expr<ForBlock> ParseForBlock();

  expected_expr<FuncCall> ParseFuncCall();
  expected_expr<Constant> ParseConstant();

  expected_expr<MacroExpr> ParseMacroExpr();
  expected_expr<TypeExpr> ParseTypeExpr();
  expected_expr<AssignmentExpr> ParseAssignmentExpr();
  expected_expr<Expr> ParseExpr();
  expected_expr<Expr::MapExpr> ParseMapExpr();
  expected_expr<Expr::CollectionExpr> ParseArrayExpr();
  expected_expr<Expr::CollectionExpr> ParseTupleExpr();
  expected_expr<PostfixExpr> ParsePostfixExpr();
  expected_expr<UnaryExpr> ParseUnaryExpr();
  expected_expr<PowerExpr> ParsePrefixExpr();
  expected_expr<MultiplicativeExpr> ParseMultiplicativeExpr();
  expected_expr<AdditiveExpr> ParseAdditiveExpr();
  expected_expr<RelationalExpr> ParseRelationalExpr();
  expected_expr<EqualityExpr> ParseEqualityExpr();
  expected_expr<LogicalAndExpr> ParseLogicalAndExpr();
  expected_expr<LogicalOrExpr> ParseLogicalOrExpr();
};

}

#endif