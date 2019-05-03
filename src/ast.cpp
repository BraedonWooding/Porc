#include "ast.hpp"

#include <type_traits>

#include "helper.hpp"

namespace porc::internals {
template<class T> struct always_false: std::false_type {};

template <class T, class... Ts>
inline constexpr bool is_any =
    std::bool_constant<(std::is_same_v<T, Ts> || ...)>::value;

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

bool FuncBlock::IsBlockExpr() const {
  if (auto expr = std::get_if<std::unique_ptr<Expr>>(&this->expr)) {
    return (*expr)->IsBlock();
  } else {
    return false;
  }
}

bool Expr::IsBlock() const {
  return std::visit([this](auto &&expr)->json {
    using T = std::decay_t<decltype(expr)>;
    if constexpr (is_any<T, Expr::FuncDecl, Expr::StructDecl,
                         std::unique_ptr<ForBlock>, std::unique_ptr<WhileBlock>,
                         std::unique_ptr<IfBlock>,
                         std::vector<std::unique_ptr<FuncBlock>>>) {
      return true;
    } else {
      return false;
    }
  }, this->expr);
}

std::optional<AssignmentOp> AssignmentOp::FromToken(Token tok) {
  switch (tok.type) {
    case Token::AddAssign: return AssignmentOp::AdditionEqual;
    case Token::SubtractAssign: return AssignmentOp::SubtractionEqual;
    case Token::Equal: return AssignmentOp::Equal;
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
    case AssignmentOp::Equal: return "=";
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

json IdentifierAccess::GetMetaData() const {
  return {
    {"name", "IdentAccess"},
    {"pos", this->pos.GetMetaData()},
    {"idents", this->idents},
    // @TODO: do representation for all objects
    // {"representation", join(this->idents, ".")}
  };
}

json FileDecl::GetMetaData() const {
  return {
    {"name", "FileDecl"},
    {"pos", this->pos.GetMetaData()},
    {"children", GetJsonForVec(exprs)}
  };
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

json StructBlock::GetMetaData() const {
  // @NOTE: this kind of visit should be really really cheap
  //        just make sure that it is
  // @PERF: from what I researched it is only when you have conditional
  //        visits that this gets expensive (i.e. perform a different operation
  //        foreach type).
  json data = std::visit([](auto &value)->json { 
    return value->GetMetaData();
  }, this->expr);

  return {
    {"name", "StructBlock"},
    {"pos", this->pos.GetMetaData()},
    {"children", data}
  };
}

json FuncBlock::GetMetaData() const {
  json data = std::visit([](auto &value)->json { 
    return value->GetMetaData();
  }, this->expr);
  if (!this->ret) return data;

  return {
    {"name", "FuncBlock"},
    {"pos", this->pos.GetMetaData()},
    {"ret", true},
    {"children", data}
  };
}

json TupleDecl::GetMetaData() const {
  std::vector<json> meta_data;
  for (auto &decl : this->args) {
    json data = {{"id", decl.id}};
    if (decl.type) data["type"] = (*decl.type)->GetMetaData();
    if (decl.expr) data["expr"] = (*decl.expr)->GetMetaData();
    data["generic"] = decl.generic_id;
    data["mut"] = decl.is_mut;
    meta_data.push_back(data);
  }
  return {
    {"name", "TupleDecl"},
    {"pos", this->pos.GetMetaData()},
    {"children", meta_data}
  };
}

json MacroExpr::GetMetaData() const {
  return {
    {"name", "MacroExpr"},
    {"pos", this->pos.GetMetaData()},
    {"id", qualifying_name->GetMetaData()},
    {"args", GetJsonForVec(args)}
  };
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

json FuncCall::GetMetaData() const {
  return {
    {"name", "FuncCall"},
    {"pos", this->pos.GetMetaData()},
    {"func", this->func->GetMetaData()},
    {"args", GetJsonForVec(args)}
  };
}

json Atom::GetMetaData() const {
  return std::visit([this](auto &&expr)->json {
    using T = std::decay_t<decltype(expr)>;
    if constexpr (std::is_same_v<T, std::unique_ptr<Expr>>) {
      return expr->GetMetaData();
    } else if constexpr (std::is_same_v<T, std::unique_ptr<MacroExpr>>) {
      return expr->GetMetaData();
    } else if constexpr (std::is_same_v<T, std::unique_ptr<Constant>>) {
      return expr->GetMetaData();
    } else if constexpr (std::is_same_v<T, std::unique_ptr<FuncCall>>) {
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
      if (expr.start) data["start"] = expr.start.value()->GetMetaData();
      if (expr.stop) data["stop"] = expr.stop.value()->GetMetaData();
      if (expr.step) data["step"] = expr.step.value()->GetMetaData();
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

json Expr::GetMetaData() const {
  return std::visit([this](auto &&expr)->json {
    using T = std::decay_t<decltype(expr)>;
    if constexpr (is_any<T, std::unique_ptr<LogicalOrExpr>,
                std::unique_ptr<WhileBlock>, std::unique_ptr<ForBlock>,
                std::unique_ptr<IfBlock>, std::unique_ptr<VarDecl>>) {
      return expr->GetMetaData();
    } else if constexpr (std::is_same_v<T,
                        std::vector<std::unique_ptr<FuncBlock>>>) {
      return {
        {"name", "FuncBlock"},
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
    } else if constexpr (std::is_same_v<T, Expr::StructDecl>) {
      return {
        {"name", "StructDecl"},
        {"pos", this->pos.GetMetaData()},
        {"members", expr.members->GetMetaData()},
        {"block", GetJsonForVec(expr.block)}
      };
    } else if constexpr (std::is_same_v<T, Expr::RangeExpr>) {
      json data = {
        {"name", "RangeExpr"},
        {"pos", this->pos.GetMetaData()},
      };
      data["start"] = expr.start->GetMetaData();
      data["stop"] = expr.stop->GetMetaData();
      data["inclusive"] = expr.inclusive;
      if (expr.step) data["step"] = (*expr.step)->GetMetaData();
      return data;
    } else if constexpr (std::is_same_v<T, Expr::MapExpr>) {
      return {
        {"name", "MapExpr"},
        {"pos", this->pos.GetMetaData()},
        {"keys", GetJsonForVec(expr.keys)},
        {"values", GetJsonForVec(expr.values)}
      };
    } else if constexpr (std::is_same_v<T, Expr::CollectionExpr>) {
      return {
        {"name", "FuncBlock"},
        {"pos", this->pos.GetMetaData()},
        {"is_array", expr.IsArray()},
        {"values", GetJsonForVec(expr.values)}
      };
    } else {
      static_assert(always_false<T>::value, "non-exhaustive vistor!");
    }
  }, this->expr);
}

json WhileBlock::GetMetaData() const {
  return {
    {"name", "While"},
    {"pos", this->pos.GetMetaData()},
    {"expr", this->expr->GetMetaData()},
    {"block", GetJsonForVec(this->block)}
  };
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

json TypeExpr::GetMetaData() const {
  return std::visit([this](auto &&expr)->json {
    using T = std::decay_t<decltype(expr)>;
    if constexpr (std::is_same_v<T, std::unique_ptr<TupleDecl>>) {
      return {
        {"name", "TypeExpr"},
        {"pos", this->pos.GetMetaData()},
        {"children", expr->GetMetaData()}
      };
    } else if constexpr (std::is_same_v<T, TypeExpr::GenericType>) {
      json arg_data = json::array();
      for (auto &arg: expr.args) {
        arg_data.push_back(std::visit([this](auto &&expr)->json {
          return expr->GetMetaData();
        }, arg));
      }

      return {
        {"name", "TypeExpr"},
        {"pos", this->pos.GetMetaData()},
        {"arg_data", std::move(arg_data)},
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
      json data = {
        {"name", "TypeExpr"},
        {"pos", this->pos.GetMetaData()}
      };
      if (expr.id) data["id"] = *expr.id;
      data["args"] = expr.args->GetMetaData();
      if (expr.ret_type) data["ret_type"] = (*expr.ret_type)->GetMetaData();

      return data;
    } else if constexpr (std::is_same_v<T, TypeExpr::GenericId>) {
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
