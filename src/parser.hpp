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

  bool ConsumeToken(Token::Kind type);

  std::optional<std::vector<LineStr>>
    ParseIdentifierAccess(Token::Kind continuer);

  std::unique_ptr<Expr> ConvIdentToExpr(LineStr id);
  std::unique_ptr<Expr> ParenthesiseExpr(std::unique_ptr<Expr> expr);

  optional_unique_ptr<Expr> ParseExprFuncOrStruct(std::unique_ptr<TupleDecl> decl);

  /*

    These could be improved, I haven't yet been able to find a method to do this
    my belief is that I can use fold expressions to partially eval all them
    But I don't know if it is possible.

  */

  /*
    We have to do the parse lists this way so we get nicer syntax for using them
    else it'll complain that `PushBack` doesn't have a inferred type since it
    can't infer it (cause the abstraction causes the compiler to not understand
    how they are related), we fix this by having this InnerType be the return
    value explicitly (rather than a template return type), then also have to
    make args parsed by reference otherwise it'll complain since it won't match

    If you get some weird error about `type` not existing it just means you
    stuffed up the type parsing make sure things are by reference like Args&...
    rather than just Args...

    Also I need to duplicate the ForEach since well one it is simpler and two
    I don't know how to do it and not conflict with the fact that args are
    variadic, if you do know how give it a go!
  */
  template<typename Fn>
  using ForEachInnerType = typename std::invoke_result_t<Fn, Parser>::value_type;

  template<typename Fn, typename ...Args>
  using ForEach1 = void(*)
    (int, ForEachInnerType<Fn>, Args&... args);

  template<typename Fn, typename ...Args>
  using ForEach2 = void(*)
    (int, ForEachInnerType<Fn>, ForEachInnerType<Fn>, Args&... args);

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
    Simple common wrappers around common operations.
  */
  template<typename Inner>
  static void PushBack(int index, Inner obj, std::vector<Inner> &vec) {
    vec.push_back(std::move(obj));
  }

  template<typename Inner>
  static void PushBackMap(int index, Inner key, Inner val,
                          std::vector<Inner> &keys, std::vector<Inner> &vals) {
    keys.push_back(std::move(key));
    vals.push_back(std::move(val));
  }

  template<typename Inner>
  static void SetIndex(int index, Inner obj, std::vector<Inner> &vec) {
    Assert(index >= 0 && index <= vec.size(),
           "Index has to be within bounds", index);
    vec[index] = std::move(obj);
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

  optional_unique_ptr<VarDecl> ParseFuncDecl();
  optional_unique_ptr<VarDecl> ParseStructDecl();

  optional_unique_ptr<FileDecl> ParseFileDecl();
  optional_unique_ptr<TupleDecl> ParseTupleDecl();
  optional_unique_ptr<VarDecl> ParseVarDecl();
  optional_unique_ptr<VarDecl>
    ParseVarDeclWithDeclList(std::vector<VarDecl::Declaration> lhs);

  optional_unique_ptr<StructBlock> ParseStructBlock();
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