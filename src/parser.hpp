#ifndef PARSER_HPP
#define PARSER_HPP

#include <type_traits>

#include "token_stream.hpp"
#include "ast.hpp"
#include "err_stream.hpp"

namespace porc::internals {

static const char *DoubleReturnErrMsg =
    "Can't have a return followed by another return.\n"
    "This applies to implicit returns as well.";

template<typename T>
using optional_unique_ptr  = std::optional<std::unique_ptr<T>>;

class Parser {
private:
  TokenStream stream;
  ErrStream err;

  /*
    Consumes a token if it matches the given type returning true.
    Else it prints out an error and returns false.
  */
  bool ConsumeToken(Token::Kind type);

  /*
    Parses a sequence of identifiers with the given token kind being the sep.
    Returns std::nullopt if there is not a ident or some other kind of syntax
    error and prints out the err msg.
  */
  optional_unique_ptr<IdentifierAccess>
    ParseIdentifierAccess(Token::Kind continuer);

  /*
    Is a safe not error printing alternative to ParseConstant.
    Tries to parse a constant and if it can't it'll return std::nullopt.
  */
  optional_unique_ptr<Constant> TryParseConstant();

  /*
    Wraps up an fold expr so that it fits as a standard expr.
  */
  std::unique_ptr<Expr> ExprToFold(std::unique_ptr<Expr> expr,
                                   bool folding_right, LineRange pos,
                                   std::unique_ptr<Atom> func);
  
  /*
    Converts an identifier to an expr.
  */
  std::unique_ptr<Expr> ConvIdentToExpr(LineStr id);

  /*
    Places the given expr inside parentheses.
  */
  std::unique_ptr<Expr> ParenthesiseExpr(std::unique_ptr<Expr> expr);

  optional_unique_ptr<Expr> ParseExprFuncOrStruct(std::unique_ptr<TupleDecl> decl);

  /*
    We have to do the parse lists this way so we get nicer syntax for using them
    else it'll complain that `PushBack` doesn't have a inferred type since it
    can't infer it (cause the abstraction causes the compiler to not understand
    how they are related), we fix this by having this InnerType be the return
    value explicitly (rather than a template return type), then also have to
    make args parsed by reference otherwise it'll complain since it won't match

    NOTE: If when using this you get errors about types not existing then
          you just need to make sure that all arg objects are passed by
          reference and not moved! i.e. Args&... args
  */
  template<typename Fn>
  using ForEachInnerType = typename std::invoke_result_t<Fn, Parser>::value_type;

  /*
    Since args are variadic I don't think I can generalize this.
    I don't need it to be generalized anyways so this is just an easy way.
  */
  template<typename Fn, typename ...Args>
  using ForEach1 = void(*)
    (int index, ForEachInnerType<Fn> a, Args&... args);

  template<typename Fn, typename ...Args>
  using ForEach2 = void(*)
    (int index, ForEachInnerType<Fn> a, ForEachInnerType<Fn> b, Args&... args);

  /*
    Parses a list of objects where there are pairs of two objects (of same type)
    separated by next_pair and in each pair the objects are separated by
    pair_sep.

    Note: The extra args passed in aren't forwarded by move constructors but
          rather should be parsed in by reference.
  */
  template<Token::Kind pair_sep = Token::Colon,
           Token::Kind next_pair = Token::Comma,
           typename Fn, typename ...Args>
  bool ParseListConjugate(Fn fn, ForEach2<Fn, Args...> for_each, Args&... args) {
    int index = 0;
    while (true) {
      auto first = (this->*fn)();
      if (!first) return false;
      if (!ConsumeToken(pair_sep)) return false;
      auto second = (this->*fn)();
      if (!second) return false;
      for_each(index, std::move(*first), std::move(*second), args...);
      if (stream.PeekCur().type == next_pair) {
        index++;
        stream.PopCur();
      } else {
        break;
      }
    }
    return true;
  }

  /*
    Parses a list of objects with a continuer token.
    Calling the ForEach method with an index (starts at 0 increments for each
    parsed item), the item parsed and forwards any args given.
    Note: the args are parsed in by referenced and AREN'T std::forward'd
          this is because they have to be passed in for each item.
          your for_each function should take references for the other args!!!
  */
  template<Token::Kind continuer = Token::Comma, typename Fn, typename ...Args>
  bool ParseList(Fn fn, ForEach1<Fn, Args...> for_each, Args&... args) {
    int index = 0;
    while (true) {
      auto res = (this->*fn)();
      if (!res) return false;
      for_each(index, std::move(*res), args...);
      if (stream.PeekCur().type == continuer) {
        index++;
        stream.PopCur();
      } else {
        break;
      }
    }
    return true;
  }

  /*
    - Parses a list of objects with operators inbetween them.
    - Handles errors and return std::nullopt on error, printing out msg.
    - Requires the ObjTo object to contain a sub struct called OpExpr that
      has a constructor taking the operator and the ObjTo object as a rhs.
    - Also requires the ObjTo object to have a constructor taking the LineRange
      as well as the first ObjTo and a vector of ObjTo::OpExpr's.
    for example:
      - useful to parse additive expressions and multiplicative expressions.
  */
  template<typename OpTo, typename ObjTo, typename ParseFn>
  optional_unique_ptr<ObjTo> ParseOpList(ParseFn fn) {
    using Inner = typename ObjTo::OpExpr;
    auto lhs = (this->*fn)();
    if (!lhs) return std::nullopt;

    std::vector<Inner> exprs;
    while (true) {
      auto op = OpTo::FromToken(stream.PeekCur());
      if (!op) break;
      stream.PopCur();

      auto rhs = (this->*fn)();
      if (!rhs) return std::nullopt;
      exprs.push_back(Inner(*op, std::move(*rhs)));
    }

    if (exprs.size() == 0) {
      // this is fine but its basically a fallthrough
      LineRange pos = (*lhs)->pos;
      return std::make_unique<ObjTo>(pos, std::move(*lhs));
    } else {
      LineRange pos = LineRange((*lhs)->pos, exprs[exprs.size() - 1].rhs->pos);
      return std::make_unique<ObjTo>(pos, std::move(*lhs), std::move(exprs));
    }
  }

  /*
    Simple common wrappers around common operations.
  */

  /*
    Just a wrapper for a vector pushback
  */
  template<typename Inner>
  static void PushBack(int index, Inner obj, std::vector<Inner> &vec) {
    vec.push_back(std::move(obj));
  }

  /*
    A wrapper to set a vector at a given index.
  */
  template<typename Inner>
  static void SetIndex(int index, Inner obj, std::vector<Inner> &vec) {
    Assert(index >= 0 && index <= vec.size(),
           "Index has to be within bounds", index);
    vec[index] = std::move(obj);
  }

  /*
    A wrapper around To::FromToken(Token) that prints out a nice error message
    in the case that this fails.

    NOTE: if you want a truly conditional token cast that doesn't error out
          use To::FromToken(Token)
  */
  template<typename To>
  std::optional<To> TokenCast(Token tok);

  /*
    Parses the RHS of the var decl including operators (i.e. `:`, `::`, ...)
  */
  optional_unique_ptr<VarDecl> Parser::ParseRhsVarDecl(
    std::vector<VarDecl::Declaration> decls);

  std::optional<TupleDecl::ArgDecl> ParseTupleDeclSegment();
  optional_unique_ptr<Expr> ParseExprOrTupleDecl();
  optional_unique_ptr<TupleDecl> ParseRestTupleDeclExpr(
    std::vector<TupleDecl::ArgDecl> declarations);
  std::optional<IfBlock::IfStatement> ParseIfStatement();

  optional_unique_ptr<Atom> ParseSliceOrIndex(std::unique_ptr<Atom> lhs);
  std::optional<std::vector<std::unique_ptr<FuncBlock>>> ParseBlock();

  /*
    Parse the assignment expression based on the ids parsed from the hint of
    var decl.
  */
  optional_unique_ptr<AssignmentExpr> Parser::ParseAssignmentExprVarDeclHint(
    std::vector<LineStr> ids);

  /*
    Parses the RHS of the assignment expr including the operator.
  */
  optional_unique_ptr<AssignmentExpr> Parser::ParseRhsAssignmentExpr(
    std::vector<std::unique_ptr<Expr>> lhs);

public:
  Parser(TokenStream stream, std::ostream &out = std::cerr)
      : stream(std::move(stream)), err(out) {
    stream.ignore_comments = true;
  }

  // optional_unique_ptr<VarDecl> ParseFuncDecl();
  // optional_unique_ptr<VarDecl> ParseStructDecl();

  optional_unique_ptr<TypeDecl> ParseTypeDecl();

  optional_unique_ptr<FileDecl> ParseFileDecl();
  optional_unique_ptr<TupleDecl> ParseTupleDecl();
  optional_unique_ptr<VarDecl> ParseVarDecl();

  optional_unique_ptr<StructBlock> ParseStructBlock();
  optional_unique_ptr<FuncBlock> ParseFuncBlock(bool file_scope = false);
  optional_unique_ptr<IfBlock> ParseIfBlock();
  optional_unique_ptr<WhileBlock> ParseWhileBlock();
  optional_unique_ptr<ForBlock> ParseForBlock();

  optional_unique_ptr<FuncCall> ParseFuncCall();
  optional_unique_ptr<Constant> ParseConstant();

  optional_unique_ptr<MacroExpr> ParseMacroExpr();
  optional_unique_ptr<TypeExpr> ParseTypeExpr();
  optional_unique_ptr<AssignmentExpr> ParseAssignmentExpr();
  optional_unique_ptr<Expr> ParseExpr();

  // @TODO: Do I need these?  I'm not using them so far?? Perhaps I should
  //        factor the logic outside of ParseExpr to get a generic collection.
  // std::optional<Expr::MapExpr> ParseMapExpr();
  // std::optional<Expr::CollectionExpr> ParseArrayExpr();
  // std::optional<Expr::CollectionExpr> ParseTupleExpr();

  optional_unique_ptr<Atom> ParseAtom();
  optional_unique_ptr<PowerExpr> ParsePowerExpr();
  optional_unique_ptr<UnaryExpr> ParseUnaryExpr();
  optional_unique_ptr<MultiplicativeExpr> ParseMultiplicativeExpr();
  optional_unique_ptr<AdditiveExpr> ParseAdditiveExpr();
  optional_unique_ptr<ComparisonExpr> ParseComparisonExpr();
  optional_unique_ptr<LogicalAndExpr> ParseLogicalAndExpr();
  optional_unique_ptr<LogicalOrExpr> ParseLogicalOrExpr();
};

}

#endif