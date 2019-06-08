#include "ast.hpp"

#include <type_traits>

#include "helper.hpp"

namespace porc {

LineRange VarDecl::Declaration::GetPos() const {
  if (expr)       return LineRange(id.pos, (*expr)->pos);
  else if (type)  return LineRange(id.pos, (*type)->pos);
  else            return id.pos;
}

// @FIXME: Didn't end up using it but there is some kind of weird bug
// template<typename T>
// std::string join(std::vector<T> &args, std::string sep) {
//   if (args.size() == 0) return "";

//   // @TODO: come up with a string builder like object
//   std::string out;
//   // we possibly could reserve even more space but this is a decent
//   // approximation of how much we will use.
//   out.reserve(args.size() * (1 + sep.size()));
//   out.append(std::to_string(args[0]));

//   for (auto arg = std::begin(out) + 1, end = std::end(out);
//         arg != end; arg++) {
//     out += sep;
//     if constexpr (std::is_same_v<T, std::string>) {
//       out += *arg;
//     } else {
//       out += std::to_string(arg));
//     }
//   }

//   return out;
// }

template<class T>
json GetJsonForVec(const std::vector<T> &vec) {
  json meta_data = json::array();
  for (const auto &arg: vec) {
    meta_data.push_back(arg->GetMetaData());
  }
  return meta_data;
}

std::optional<AssignmentOp> AssignmentOp::FromToken(Token tok) {
  switch (tok.type) {
    case Token::AddAssign: return AssignmentOp::AdditionEqual;
    case Token::SubtractAssign: return AssignmentOp::SubtractionEqual;
    case Token::Assign: return AssignmentOp::Assign;
    case Token::DivideAssign: return AssignmentOp::DivisionEqual;
    case Token::PowerAssign: return AssignmentOp::PowerEqual;
    case Token::ModulusAssign: return AssignmentOp::ModulusEqual;
    case Token::MultiplyAssign: return AssignmentOp::MultiplyEqual;
    case Token::IntegerDivideAssign: return AssignmentOp::IntDivisionEqual;
    default: return std::nullopt;
  }
}

std::optional<PrefixOp> PrefixOp::FromToken(Token tok) {
  switch (tok.type) {
    case Token::Negate: return PrefixOp::Negate;
    case Token::Subtract: return PrefixOp::Negative;
    case Token::Add: return PrefixOp::Positive;
    default: return std::nullopt;
  }
}

std::optional<MultiplicativeOp> MultiplicativeOp::FromToken(Token tok) {
  switch (tok.type) {
    case Token::Multiply: return MultiplicativeOp::Multiplication;
    case Token::Divide: return MultiplicativeOp::Division;
    case Token::Modulus: return MultiplicativeOp::Modulus;
    case Token::IntegerDivide: return MultiplicativeOp::IntDivision;
    default: return std::nullopt;
  }
}

std::optional<AdditiveOp> AdditiveOp::FromToken(Token tok) {
  switch (tok.type) {
    case Token::Add: return AdditiveOp::Addition;
    case Token::Subtract: return AdditiveOp::Subtraction;
    default: return std::nullopt;
  }
}

std::optional<ComparisonOp> ComparisonOp::FromToken(Token tok) {
  switch (tok.type) {
    case Token::GreaterThan: return ComparisonOp::GreaterThan;
    case Token::GreaterThanEqual: return ComparisonOp::LessThan;
    case Token::LessThan: return ComparisonOp::GreaterThanEqual;
    case Token::LessThanEqual: return ComparisonOp::LessThanEqual;
    case Token::Equal: return ComparisonOp::Equal;
    case Token::NotEqual: return ComparisonOp::NotEqual;
    default: return std::nullopt;
  }
}

const char *AssignmentOp::ToStr() const {
  switch (value) {
    case AssignmentOp::AdditionEqual: return "+=";
    case AssignmentOp::SubtractionEqual: return "-=";
    case AssignmentOp::Assign: return "=";
    case AssignmentOp::DivisionEqual: return "/=";
    case AssignmentOp::PowerEqual: return "**=";
    case AssignmentOp::ModulusEqual: return "%=";
    case AssignmentOp::MultiplyEqual: return "*=";
    case AssignmentOp::IntDivisionEqual: return "//=";
    default: Unreachable("Unhandled case");
  }
}

const char *AssignmentOp::AllMsg() {
  return "+=, -=, =, /=, **=, %=, *=, //=";
}

const char *PrefixOp::ToStr() const {
  switch (value) {
    case PrefixOp::Negate: return "!";
    case PrefixOp::Negative: return "-";
    case PrefixOp::Positive: return "+";
    default: Unreachable("Unhandled case");
  }
}

const char *PrefixOp::AllMsg() {
  return "!, -, +";
}

const char *MultiplicativeOp::ToStr() const {
  switch (value) {
    case MultiplicativeOp::Multiplication: return "*";
    case MultiplicativeOp::Division: return "/";
    case MultiplicativeOp::Modulus: return "%";
    case MultiplicativeOp::IntDivision: return "//";
    default: Unreachable("Unhandled case");
  }
}

const char *MultiplicativeOp::AllMsg() {
  return "*, /, %, //";
}

const char *AdditiveOp::ToStr() const {
  switch (value) {
    case AdditiveOp::Addition: return "+";
    case AdditiveOp::Subtraction: return "-";
    default: Unreachable("Unhandled case");
  }
}

const char *AdditiveOp::AllMsg() {
  return "+, -";
}

const char *ComparisonOp::ToStr() const {
  switch (value) {
    case ComparisonOp::GreaterThan: return ">";
    case ComparisonOp::LessThan: return "<";
    case ComparisonOp::GreaterThanEqual: return ">=";
    case ComparisonOp::LessThanEqual: return "<=";
    case ComparisonOp::Equal: return "==";
    case ComparisonOp::NotEqual: return "!=";
    default: Unreachable("Unhandled case");
  }
}

const char *ComparisonOp::AllMsg() {
  return ">, <, >=, <=, ==, !=";
}

KindAST IdentifierAccess::UnwrapToLowest(void **ast) {
  if (idents.size() != 1) {
    if (ast) *ast = this;
    return KindAST::IdentifierAccess;
  } else {
    if (ast) *ast = &idents[0];
    return KindAST::Identifier;
  }
}

json IdentifierAccess::GetMetaData() const {
  return {
    {"name", "IdentAccess"},
    {"pos", this->pos.GetMetaData()},
    {"idents", this->idents},
    // @TODO: do representation for all objects
    // {"representation", join(this->idents, ".")}
  };
}

/*
  @TODO: Should we resolve it to an expr if only one expr (and no types) exist?
         Vice versa for types?
  Could make some things easier I guess...????
*/
KindAST FileDecl::UnwrapToLowest(void **ast) {
  if (exprs.size() == 1 && types.size() == 0) {
    return exprs[0]->UnwrapToLowest(ast);
  } else if (exprs.size() == 0 && types.size() == 1) {
    return types[0]->UnwrapToLowest(ast);
  } else {
    if (ast) *ast = this;
    return KindAST::FileDecl;
  }
}

json FileDecl::GetMetaData() const {
  return {
    {"name", "FileDecl"},
    {"pos", this->pos.GetMetaData()},
    {"statements", GetJsonForVec(exprs)},
    {"types", GetJsonForVec(types)},
  };
}

KindAST VarDecl::UnwrapToLowest(void **ast) {
  if (decls.size() == 1) {
    if (ast) *ast = &decls[0];
    return KindAST::VarDeclDeclaration;
  } else {
    if (ast) *ast = this;
    return KindAST::VarDecl;
  }
}

json VarDecl::GetMetaData() const {
  std::vector<json> meta_data;
  for (auto &decl : this->decls) {
    json data;
    data["id"] = decl.id;
    if (decl.type) data["type"] = (*decl.type)->GetMetaData();
    if (decl.expr) data["expr"] = (*decl.expr)->GetMetaData();
    meta_data.push_back(data);
  }
  return {
    {"name", "VarDecl"},
    {"pos", this->pos.GetMetaData()},
    {"mut", this->is_mut},
    {"children", meta_data}
  };
}

KindAST TypeStatement::UnwrapToLowest(void **ast) {
  return std::visit([this, ast](auto &&expr)->KindAST {
    using T = std::decay_t<decltype(expr)>;
    if constexpr (is_any<T, std::unique_ptr<TypeDecl>,
                            std::unique_ptr<MacroExpr>>) {
      return expr->UnwrapToLowest(ast);
    } else if constexpr (std::is_same_v<T, Declaration>) {
      if (expr.access) {
        if (ast) *ast = static_cast<Declaration*>(&expr);
        return KindAST::TypeStatementDeclaration;
      } else {
        return expr.decl->UnwrapToLowest(ast);
      }
    } else {
      static_assert(always_false<T>::value, "non-exhaustive vistor!");
    }
  }, this->expr);
}

json TypeStatement::GetMetaData() const {
  return std::visit([this](auto &&expr)->json {
    using T = std::decay_t<decltype(expr)>;
    if constexpr (is_any<T, std::unique_ptr<TypeDecl>,
                            std::unique_ptr<MacroExpr>>) {
      return expr->GetMetaData();
    } else if constexpr (std::is_same_v<T, Declaration>) {
      json data = {
        {"name", "TypeStatement"},
        {"pos", pos.GetMetaData()},
        {"decl", expr.decl->GetMetaData()}
      };
      if (expr.access) data["access"] = (*expr.access)->GetMetaData();
      return data;
    } else {
      static_assert(always_false<T>::value, "non-exhaustive vistor!");
    }
  }, this->expr);
}

KindAST FuncStatement::UnwrapToLowest(void **ast) {
  if (HasPrefix(NoPrefix)) {
    return std::visit([ast](auto &value)->KindAST { 
      return value->UnwrapToLowest(ast);
    }, this->expr);
  } else {
    if (ast) *ast = this;
    return KindAST::FuncStatement;
  }
}

json FuncStatement::GetMetaData() const {
  json data = std::visit([](auto &value)->json { 
    return value->GetMetaData();
  }, this->expr);

  if (HasPrefix(NoPrefix)) return data;

  std::string prefix = HasPrefix(Yield) ? "yield " : "";
  switch (this->prefix & ~Yield) {
    case NoPrefix:  prefix = HasPrefix(Yield) ? "yield" : ""; break;
    case Return:    prefix += "return"; break;
    case Continue:  prefix += "continue"; break;
    case Break:     prefix += "break"; break;
    case BlockVal:  prefix += "="; break;
  }

  return {
    {"name", "FuncStatement"},
    {"pos", this->pos.GetMetaData()},
    {"prefix", prefix},
    {"children", data}
  };
}

KindAST TupleValueDecl::UnwrapToLowest(void **ast) {
  if (ast) *ast = this;
  return KindAST::TupleValueDecl;
}

json TupleValueDecl::GetMetaData() const {
  std::vector<json> meta_data;
  for (auto &decl : this->args) {
    json data = {{"id", decl.id}};
    if (decl.type) data["type"] = (*decl.type)->GetMetaData();
    if (decl.expr) data["expr"] = (*decl.expr)->GetMetaData();
    meta_data.push_back(data);
  }
  return {
    {"name", "TupleValueDecl"},
    {"pos", this->pos.GetMetaData()},
    {"children", meta_data}
  };
}

KindAST TupleTypeDecl::UnwrapToLowest(void **ast) {
  if (ast) *ast = this;
  return KindAST::TupleTypeDecl;
}

json TupleTypeDecl::GetMetaData() const {
  std::vector<json> meta_data;
  for (auto &decl : this->args) {
    json data = {{"type", decl.type->GetMetaData()}};
    if (decl.id) data["id"] = (*decl.id);
    meta_data.push_back(data);
  }
  return {
    {"name", "TupleTypeDecl"},
    {"pos", this->pos.GetMetaData()},
    {"children", meta_data}
  };
}

KindAST MacroExpr::UnwrapToLowest(void **ast) {
  if (ast) *ast = this;
  return KindAST::MacroExpr;
}

json MacroExpr::GetMetaData() const {
  return {
    {"name", "MacroExpr"},
    {"pos", this->pos.GetMetaData()},
    {"id", qualifying_name->GetMetaData()},
    {"args", GetJsonForVec(args)}
  };
}

KindAST AssignmentExpr::UnwrapToLowest(void **ast) {
  if (ast) *ast = this;
  return KindAST::AssignmentExpr;
}

json AssignmentExpr::GetMetaData() const {
  return {
    {"name", "AssignmentExpr"},
    {"pos", this->pos.GetMetaData()},
    {"lhs", GetJsonForVec(lhs)},
    {"op", op.ToStr()},
    {"rhs", GetJsonForVec(rhs)}
  };
}

KindAST FuncCall::UnwrapToLowest(void **ast) {
  if (ast) *ast = this;
  return KindAST::FuncCall;
}

json FuncCall::GetMetaData() const {
  return {
    {"name", "FuncCall"},
    {"pos", this->pos.GetMetaData()},
    {"func", this->func->GetMetaData()},
    {"args", GetJsonForVec(args)}
  };
}

KindAST Atom::UnwrapToLowest(void **ast) {
  return std::visit([this, ast](auto &&expr)->KindAST {
    using T = std::decay_t<decltype(expr)>;
    if constexpr (is_any<T, std::unique_ptr<Expr>, std::unique_ptr<MacroExpr>,
                       std::unique_ptr<Constant>, std::unique_ptr<FuncCall>>) {
      return expr->UnwrapToLowest(ast);
    } else if constexpr (std::is_same_v<T, LineStr>) {
      if (ast) *ast = static_cast<LineStr*>(&expr);
      return KindAST::Identifier;
    } else if constexpr (std::is_same_v<T, Atom::IndexExpr>) {
      if (ast) *ast = static_cast<Atom::IndexExpr*>(&expr);
      return KindAST::AtomIndexExpr;
    } else if constexpr (std::is_same_v<T, Atom::SliceExpr>) {
      if (ast) *ast = static_cast<Atom::SliceExpr*>(&expr);
      return KindAST::AtomSliceExpr;
    } else if constexpr (std::is_same_v<T, Atom::FoldExpr>) {
      if (ast) *ast = static_cast<Atom::FoldExpr*>(&expr);
      return KindAST::AtomFoldExpr;
    } else if constexpr (std::is_same_v<T, Atom::MemberAccessExpr>) {
      if (ast) *ast = static_cast<Atom::MemberAccessExpr*>(&expr);
      return KindAST::AtomMemberAccessExpr;
    } else {
      static_assert(always_false<T>::value, "non-exhaustive vistor!");
    }
  }, this->expr);
}

json Atom::GetMetaData() const {
  return std::visit([this](auto &&expr)->json {
    using T = std::decay_t<decltype(expr)>;
    if constexpr (is_any<T, std::unique_ptr<Expr>, std::unique_ptr<MacroExpr>,
                       std::unique_ptr<Constant>, std::unique_ptr<FuncCall>>) {
      return expr->GetMetaData();
    } else if constexpr (std::is_same_v<T, LineStr>) {
      return {
        {"name", "Identifier"},
        {"pos", expr.pos.GetMetaData()},
        {"children", expr}
      };
    } else if constexpr (std::is_same_v<T, Atom::IndexExpr>) {
      return {
        {"name", "IndexExpr"},
        {"pos", this->pos.GetMetaData()},
        {"lhs", expr.lhs->GetMetaData()},
        {"index", expr.index->GetMetaData()}
      };
    } else if constexpr (std::is_same_v<T, Atom::SliceExpr>) {
      json data = {
        {"name", "SliceExpr"},
        {"pos", this->pos.GetMetaData()},
        {"obj", expr.obj->GetMetaData()}
      };
      if (expr.start) data["start"] = (*expr.start)->GetMetaData();
      if (expr.stop) data["stop"] = (*expr.stop)->GetMetaData();
      if (expr.step) data["step"] = (*expr.step)->GetMetaData();
      return data;
    } else if constexpr (std::is_same_v<T, Atom::FoldExpr>) {
      return {
        {"name", "FoldExpr"},
        {"pos", this->pos.GetMetaData()},
        {"folds_right", expr.folds_right},
        {"lhs", expr.func->GetMetaData()},
        {"fold_expr", expr.fold_expr->GetMetaData()}
      };
    } else if constexpr (std::is_same_v<T, Atom::MemberAccessExpr>) {
      return {
        {"name", "MemberAccessExpr"},
        {"pos", this->pos.GetMetaData()},
        {"lhs", expr.lhs->GetMetaData()},
        {"member", expr.access->GetMetaData()}
      };
    } else {
      static_assert(always_false<T>::value, "non-exhaustive vistor!");
    }
  }, this->expr);
}

KindAST PowerExpr::UnwrapToLowest(void **ast) {
  if (exprs.size() > 1) {
    if (ast) *ast = this;
    return KindAST::PowerExpr;
  } else {
    return exprs[1]->UnwrapToLowest(ast);
  }
}

json PowerExpr::GetMetaData() const {
  if (exprs.size() == 1) {
    // fallthrough
    return exprs[0]->GetMetaData();
  } else {
    return {
      {"name", "PowerExpr"},
      {"pos", this->pos.GetMetaData()},
      {"children", GetJsonForVec(exprs)}
    };
  }
}

KindAST TypeDecl::UnwrapToLowest(void **ast) {
  if (ast) *ast = this;
  return KindAST::TypeDecl;
}

json TypeDecl::GetMetaData() const {
  json data = {
    {"name", "TypeDecl"},
    {"pos", this->pos.GetMetaData()},
    {"id", this->id},
    {"block", GetJsonForVec(block)}
  };
  if (type) data["type"] = (*type)->GetMetaData();
  return data;
}

KindAST UnaryExpr::UnwrapToLowest(void **ast) {
  if (ops.size() > 1) {
    if (ast) *ast = this;
    return KindAST::UnaryExpr;
  } else {
    return rhs->UnwrapToLowest(ast);
  }
}

json UnaryExpr::GetMetaData() const {
  if (ops.size() == 0) {
    // fallthrough
    return rhs->GetMetaData();
  } else {
    json data;
    for (const auto &op : ops) {
      data.push_back(op.ToStr());
    }

    return {
      {"name", "MultiplicativeExpr"},
      {"pos", this->pos.GetMetaData()},
      {"rhs", rhs->GetMetaData()},
      {"ops", data}
    };
  }
}

KindAST MultiplicativeExpr::UnwrapToLowest(void **ast) {
  if (exprs.size() > 1) {
    if (ast) *ast = this;
    return KindAST::MultiplicativeExpr;
  } else {
    return lhs->UnwrapToLowest(ast);
  }
}

json MultiplicativeExpr::GetMetaData() const {
  if (exprs.size() == 0) {
    // fallthrough
    return lhs->GetMetaData();
  } else {
    json data;
    for (const auto &expr : exprs) {
      data.push_back({{"op", expr.op.ToStr()},
                      {"rhs", expr.rhs->GetMetaData()}});
    }

    return {
      {"name", "MultiplicativeExpr"},
      {"pos", this->pos.GetMetaData()},
      {"lhs", lhs->GetMetaData()},
      {"ops", data}
    };
  }
}

KindAST AdditiveExpr::UnwrapToLowest(void **ast) {
  if (exprs.size() > 1) {
    if (ast) *ast = this;
    return KindAST::AdditiveExpr;
  } else {
    return lhs->UnwrapToLowest(ast);
  }
}

json AdditiveExpr::GetMetaData() const {
  if (exprs.size() == 0) {
    // fallthrough
    return lhs->GetMetaData();
  } else {
    json data;
    for (const auto &expr : exprs) {
      data.push_back({{"op", expr.op.ToStr()},
                      {"rhs", expr.rhs->GetMetaData()}});
    }

    return {
      {"name", "AdditiveExpr"},
      {"pos", this->pos.GetMetaData()},
      {"lhs", lhs->GetMetaData()},
      {"ops", data}
    };
  }
}

KindAST ComparisonExpr::UnwrapToLowest(void **ast) {
  if (exprs.size() > 1) {
    if (ast) *ast = this;
    return KindAST::ComparisonExpr;
  } else {
    return lhs->UnwrapToLowest(ast);
  }
}

json ComparisonExpr::GetMetaData() const {
  if (exprs.size() == 0) {
    // fallthrough
    return lhs->GetMetaData();
  } else {
    json data;
    for (const auto &expr : exprs) {
      data.push_back({{"op", expr.op.ToStr()},
                      {"rhs", expr.rhs->GetMetaData()}});
    }

    return {
      {"name", "ComparisonExpr"},
      {"pos", this->pos.GetMetaData()},
      {"lhs", lhs->GetMetaData()},
      {"ops", data}
    };
  }
}

KindAST LogicalAndExpr::UnwrapToLowest(void **ast) {
  if (exprs.size() > 1) {
    if (ast) *ast = this;
    return KindAST::LogicalAndExpr;
  } else {
    return exprs[0]->UnwrapToLowest(ast);
  }
}

json LogicalAndExpr::GetMetaData() const {
  if (exprs.size() == 1) {
    // fallthrough
    return exprs[0]->GetMetaData();
  } else {
    return {
      {"name", "LogicalAndExpr"},
      {"pos", this->pos.GetMetaData()},
      {"children", GetJsonForVec(exprs)}
    };
  }
}

KindAST LogicalOrExpr::UnwrapToLowest(void **ast) {
  if (exprs.size() > 1) {
    if (ast) *ast = this;
    return KindAST::LogicalOrExpr;
  } else {
    return exprs[0]->UnwrapToLowest(ast);
  }
}

json LogicalOrExpr::GetMetaData() const {
  if (exprs.size() == 1) {
    // fallthrough
    return exprs[0]->GetMetaData();
  } else {
    return {
      {"name", "LogicalOrExpr"},
      {"pos", this->pos.GetMetaData()},
      {"children", GetJsonForVec(exprs)}
    };
  }
}

KindAST Expr::UnwrapToLowest(void **ast) {
  return std::visit([this, ast](auto &&expr)->KindAST {
    using T = std::decay_t<decltype(expr)>;
    if constexpr (is_any<T, std::unique_ptr<LogicalOrExpr>,
                std::unique_ptr<WhileBlock>, std::unique_ptr<ForBlock>,
                std::unique_ptr<IfBlock>, std::unique_ptr<VarDecl>,
                std::unique_ptr<AssignmentExpr>>) {
      return expr->UnwrapToLowest(ast);
    } else if constexpr (std::is_same_v<T,
                         std::vector<std::unique_ptr<FuncStatement>>>) {
      if (expr.size() == 1) {
        return expr[0]->UnwrapToLowest(ast);
      } else {
        if (ast) *ast = static_cast<vector_unique_ptr<FuncStatement>*>(&expr);
        return KindAST::ExprBlock;
      }
    } else if constexpr (std::is_same_v<T, Expr::FuncDecl>) {
      if (ast) *ast = static_cast<Expr::FuncDecl*>(&expr);
      return KindAST::ExprFuncDecl;
    } else if constexpr (std::is_same_v<T, Expr::RangeExpr>) {
      if (ast) *ast = static_cast<Expr::RangeExpr*>(&expr);
      return KindAST::ExprRangeExpr;
    } else if constexpr (std::is_same_v<T, Expr::CollectionExpr>) {
      if (ast) *ast = static_cast<Expr::CollectionExpr*>(&expr);
      return KindAST::ExprCollectionExpr;
    } else {
      static_assert(always_false<T>::value, "non-exhaustive vistor!");
    }
  }, this->expr);
}

json Expr::GetMetaData() const {
  return std::visit([this](auto &&expr)->json {
    using T = std::decay_t<decltype(expr)>;
    if constexpr (is_any<T, std::unique_ptr<LogicalOrExpr>,
                std::unique_ptr<WhileBlock>, std::unique_ptr<ForBlock>,
                std::unique_ptr<IfBlock>, std::unique_ptr<VarDecl>,
                std::unique_ptr<AssignmentExpr>>) {
      return expr->GetMetaData();
    } else if constexpr (std::is_same_v<T,
                        std::vector<std::unique_ptr<FuncStatement>>>) {
      return {
        {"name", "FuncStatement"},
        {"pos", this->pos.GetMetaData()},
        {"children", GetJsonForVec(expr)}
      };
    } else if constexpr (std::is_same_v<T, Expr::FuncDecl>) {
      json data = {
        {"name", "FuncDecl"},
        {"pos", this->pos.GetMetaData()},
        {"args", expr.args->GetMetaData()},
        {"block", GetJsonForVec(expr.block)},
      };
      if (expr.ret_type) data["ret"] = (*expr.ret_type)->GetMetaData();
      return data;
    } else if constexpr (std::is_same_v<T, Expr::RangeExpr>) {
      json data = {
        {"name", "RangeExpr"},
        {"pos", this->pos.GetMetaData()},
      };
      if (expr.start) data["start"] = (*expr.start)->GetMetaData();
      if (expr.stop) data["stop"] = (*expr.stop)->GetMetaData();
      data["inclusive"] = expr.inclusive;
      if (expr.step) data["step"] = (*expr.step)->GetMetaData();
      return data;
    } else if constexpr (std::is_same_v<T, Expr::CollectionExpr>) {
      return {
        {"name", "FuncStatement"},
        {"pos", this->pos.GetMetaData()},
        {"is_array", expr.IsArray()},
        {"values", GetJsonForVec(expr.values)}
      };
    } else {
      static_assert(always_false<T>::value, "non-exhaustive vistor!");
    }
  }, this->expr);
}

KindAST WhileBlock::UnwrapToLowest(void **ast) {
  if (ast) *ast = this;
  return KindAST::WhileBlock;
}

json WhileBlock::GetMetaData() const {
  return {
    {"name", "While"},
    {"pos", this->pos.GetMetaData()},
    {"expr", this->expr->GetMetaData()},
    {"block", GetJsonForVec(this->block)}
  };
}

KindAST ForBlock::UnwrapToLowest(void **ast) {
  if (ast) *ast = this;
  return KindAST::ForBlock;
}

json ForBlock::GetMetaData() const {
  return {
    {"name", "For"},
    {"pos", this->pos.GetMetaData()},
    {"id_list", this->id_list->GetMetaData()},
    {"expr_list", GetJsonForVec(this->expr_list)},
    {"block", GetJsonForVec(this->block)}
  };
}

KindAST IfBlock::UnwrapToLowest(void **ast) {
  if (statements.size() == 1 && !else_block) {
    if (ast) *ast = &statements[0];
    return KindAST::IfBlockStatement;
  } else {
    if (ast) *ast = this;
    return KindAST::IfBlock;
  }
}

json IfBlock::GetMetaData() const {
  json data;
  data["statements"] = json::array();
  for (auto &statement : statements) {
    data["statements"].push_back({
      {"cond", statement.cond->GetMetaData()},
      {"block", GetJsonForVec(statement.block)}
    });
  }

  if (this->else_block) data["else"] = GetJsonForVec(*this->else_block);

  return {
    {"name", "If"},
    {"pos", this->pos.GetMetaData()},
    {"children", data}
  };
}

KindAST TypeExpr::UnwrapToLowest(void **ast) {
  return std::visit([this, ast](auto &&expr)->KindAST {
    using T = std::decay_t<decltype(expr)>;
    if constexpr (is_any<T, std::unique_ptr<TupleTypeDecl>,
                            std::unique_ptr<IdentifierAccess>>) {
      return expr->UnwrapToLowest(ast);
    } else if constexpr (std::is_same_v<T, TypeExpr::GenericType>) {
      if (ast) *ast = static_cast<TypeExpr::GenericType*>(&expr);
      return KindAST::TypeExprGeneric;
    } else if constexpr (std::is_same_v<T, TypeExpr::VariantType>) {
      if (ast) *ast = static_cast<TypeExpr::VariantType*>(&expr);
      return KindAST::TypeExprVariant;
    } else if constexpr (std::is_same_v<T, TypeExpr::FunctionType>) {
      if (ast) *ast = static_cast<TypeExpr::FunctionType*>(&expr);
      return KindAST::TypeExprFunction;
    } else if constexpr (std::is_same_v<T, LineStr>) {
      if (ast) *ast = static_cast<LineStr*>(&expr);
      return KindAST::Identifier;
    } else {
      static_assert(always_false<T>::value, "non-exhaustive vistor!");
    }
  }, this->expr);
}

json TypeExpr::GetMetaData() const {
  return std::visit([this](auto &&expr)->json {
    using T = std::decay_t<decltype(expr)>;
    if constexpr (std::is_same_v<T, std::unique_ptr<TupleTypeDecl>>) {
      return {
        {"name", "TypeExpr"},
        {"pos", this->pos.GetMetaData()},
        {"children", expr->GetMetaData()}
      };
    } else if constexpr (std::is_same_v<T, TypeExpr::GenericType>) {
      return {
        {"name", "TypeExpr"},
        {"pos", this->pos.GetMetaData()},
        {"arg_data", GetJsonForVec(expr.args)},
        {"identifier", expr.ident->GetMetaData()}
      };
    } else if constexpr (std::is_same_v<T, TypeExpr::VariantType>) {
      return {
        {"name", "TypeExpr"},
        {"pos", this->pos.GetMetaData()},
        {"lhs", expr.lhs->GetMetaData()},
        {"rhs", GetJsonForVec(expr.rhs)}
      };
    } else if constexpr (std::is_same_v<T, TypeExpr::FunctionType>) {
      return {
        {"name", "TypeExpr"},
        {"pos", this->pos.GetMetaData()},
        {"args", expr.args->GetMetaData()},
        {"ret_type", expr.ret_type->GetMetaData()}
      };
    } else if constexpr (std::is_same_v<T, LineStr>) {
      return {
        {"name", "TypeExpr"},
        {"pos", this->pos.GetMetaData()},
        {"children", expr}
      };
    } else if constexpr (std::is_same_v<T, std::unique_ptr<IdentifierAccess>>) {
      return {
        {"name", "TypeExpr"},
        {"pos", this->pos.GetMetaData()},
        {"children", expr->GetMetaData()}
      };
    } else {
      static_assert(always_false<T>::value, "non-exhaustive vistor!");
    }
  }, this->expr);
}

KindAST Constant::UnwrapToLowest(void **ast) {
  return std::visit([this, ast](auto &&expr)->KindAST {
    using T = std::decay_t<decltype(expr)>;
    if constexpr (std::is_same_v<T, double>) {
      if (ast) *ast = static_cast<double*>(&expr);
      return KindAST::Double;
    } else if constexpr (std::is_same_v<T, i64>) {
      if (ast) *ast = static_cast<i64*>(&expr);
      return KindAST::Int;
    } else if constexpr (std::is_same_v<T, std::string>) {
      if (ast) *ast = static_cast<std::string*>(&expr);
      return KindAST::String;
    } else if constexpr (std::is_same_v<T, char>) {
      if (ast) *ast = static_cast<char*>(&expr);
      return KindAST::Character;
    } else if constexpr (std::is_same_v<T, bool>) {
      if (ast) *ast = static_cast<bool*>(&expr);
      return KindAST::Boolean;
    } else {
      static_assert(always_false<T>::value, "non-exhaustive vistor!");
    }
  }, this->data);
}

json Constant::GetMetaData() const {
  return std::visit([this](auto &&expr)->json {
    using T = std::decay_t<decltype(expr)>;
    if constexpr (is_any<T, double, i64, std::string, char, bool>) {
      return {
        {"name", "Constant"},
        {"pos", this->pos.GetMetaData()},
        {"value", expr}
      };
    } else {
      static_assert(always_false<T>::value, "non-exhaustive vistor!");
    }
  }, this->data);
}

}
