#include "ast.hpp"

#include <type_traits>

#include "helper.hpp"

namespace porc::internals {
  LineRange VarDecl::Declaration::GetPos() const {
    if (expr)       return LineRange(id.pos, (*expr)->pos);
    else if (type)  return LineRange(id.pos, (*type)->pos);
    else            return id.pos;
  }

  template<typename T>
  std::string join(const T &args, std::string sep) {
    if (args.size() == 0) return "";

    // @TODO: come up with a string builder like object
    std::string out;
    // we possibly could reserve even more space but this is a decent
    // approximation of how much we will use.
    out.reserve(args.size() * (1 + sep.size()));
    out.append(std::to_string(args[0]));

    for (auto &arg = std::begin(out) + 1, end = std::end(out);
         arg != end; arg++) {
      out.append(sep);
      out.append(std::to_string(arg));
    }

    return out;
  }
}

template<class T>
std::vector<json> GetJsonForVec(const std::vector<T> &vec) {
  json meta_data;
  for (const auto &arg: vec) {
    meta_data.emplace(arg->GetMetaData());
  }
  return meta_data;
}

template<class T> struct always_false: std::false_type {};

template <class T, class... Ts>
inline constexpr bool is_any =
    std::bool_constant<(std::is_same_v<T, Ts> || ...)>::value;

namespace porc::internals {

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
  return ">, <, >=, <=", "==, !=";
}

json IdentifierAccess::GetMetaData() const {
  return {
    {"name", "IdentAccess"},
    {"pos", this->pos.GetMetaData()},
    {"data", this->idents},
    // @TODO: do representation for all objects
    // {"representation", join(this->idents, ".")}
  };
}

json FileDecl::GetMetaData() const {
  return {
    {"name", "FileDecl"},
    {"pos", this->pos.GetMetaData()},
    {"data", GetJsonForVec(exprs)}
  };
}

json VarDecl::GetMetaData() const {
  std::vector<json> meta_data;
  for (auto &decl : this->decls) {
    json data = {"id", decl.id};
    if decl.type->GetMetaData() data["type"] = (*decl.type)->GetMetaData();
    if decl.expr->GetMetaData() data["expr"] = (*decl.expr)->GetMetaData();
    meta_data.push_back(data);
  }
  return {
    {"name", "VarDecl"},
    {"pos", this->pos.GetMetaData()},
    {"data", {
      {"mut", this->is_mut},
      {"var_data", meta_data}
    }}
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
    {"data", data}
  };
}

json FuncBlock::GetMetaData() const {
  json data = std::visit([](auto &value)->json { 
    return value->GetMetaData();
  }, this->expr);
  if (!this->ret) return data;

  // state we return since this->ret == true
  data.emplace("ret", true);
  return {
    {"name", "FuncBlock"},
    {"pos", this->pos.GetMetaData()},
    {"data", data}
  };
}

json TupleDecl::GetMetaData() const {
  std::vector<json> meta_data;
  for (auto &decl : this->args) {
    json data = {"id", decl.id};
    if decl.type->GetMetaData() data["type"] = (*decl.type)->GetMetaData();
    if decl.expr->GetMetaData() data["expr"] = (*decl.expr)->GetMetaData();
    data["generic"] = decl.generic;
    data["mut"] = decl.is_mut;
    meta_data.push_back(data);
  }
  return {
    {"name", "TupleDecl"},
    {"pos", this->pos.GetMetaData()},
    {"data", {
      {"mut", this->is_mut},
      {"var_data", meta_data}
    }}
  };
}

json MacroExpr::GetMetaData() const {
  return {
    {"name", "MacroExpr"},
    {"pos", this->pos.GetMetaData()},
    {"data", {
      {"id", qualifying_name->GetMetaData()},
      {"args", GetJsonForVec(args)}
    }}
  };
}

json AssignmentExpr::GetMetaData() const {
  return {
    {"name", name},
    {"pos", this->pos.GetMetaData()},
    {"data", {
      {"lhs", GetJsonForVec(lhs)},
      {"op", op.ToStr()},
      {"rhs", GetJsonForVec(rhs)}
    }}
  };
}

json FuncCall::GetMetaData() const {
  json data;
  data["func"] = this->func->GetMetaData();
  data["args"] = GetJsonForVec(args);

  return {
    {"name", "FuncCall"},
    {"pos", this->pos.GetMetaData()},
    {"data", data}
  };
}

// json PrimaryExpr::GetMetaData() const {
//   // else just return the json stuff for the sub value
//   // no need to present primary stuff
//   return std::visit([](auto &&expr)->json {
//     using T = std::decay_t<decltype(expr)>;
//     if constexpr (std::is_same_v<T, std::string>) return expr;
//     else return expr->GetMetaData(); 
//   }, this->expr);
// }


// json PostfixExpr::GetMetaData() const {
//   return std::visit([this](auto &&expr)->json {
//     using T = std::decay_t<decltype(expr)>;
//     if constexpr (std::is_same_v<T, std::unique_ptr<PrimaryExpr>>) {
//       return expr->GetMetaData();
//     } else if constexpr (std::is_same_v<T, std::unique_ptr<FuncCall>>) {
//       return expr->GetMetaData();
//     } else if constexpr (std::is_same_v<T, PostfixExpr::IndexExpr>) {
//       return {
//         {"name", "IndexExpr"},
//         {"pos", this->pos.GetMetaData()},
//         {"data", {
//           {"lhs", expr.lhs->GetMetaData()},
//           {"index", expr.index->GetMetaData()}
//         }}
//       };
//     } else if constexpr (std::is_same_v<T, PostfixExpr::SliceExpr>) {
//       json data = {{"obj", expr.obj->GetMetaData()}};
//       if (expr.start) data["start"] = expr.start.value()->GetMetaData();
//       if (expr.stop) data["stop"] = expr.stop.value()->GetMetaData();
//       if (expr.step) data["step"] = expr.step.value()->GetMetaData();

//       return {
//         {"name", "SliceExpr"},
//         {"pos", this->pos.GetMetaData()},
//         {"data", data}
//       };
//     } else if constexpr (std::is_same_v<T, PostfixExpr::FoldExpr>) {
//       return {
//         {"name", "FoldExpr"},
//         {"pos", this->pos.GetMetaData()},
//         {"data", {
//           {"func", expr.func->GetMetaData()},
//           {"fold_expr", expr.fold_expr->GetMetaData()}
//         }}
//       };
//     } else if constexpr (std::is_same_v<T, PostfixExpr::PostfixOpExpr>) {
//       return {
//         {"name", "PostfixOpExpr"},
//         {"pos", this->pos.GetMetaData()},
//         {"data", {
//           {"lhs", expr.lhs->GetMetaData()},
//           {"op", expr.op.ToStr()}
//         }}
//       };
//     } else if constexpr (std::is_same_v<T, PostfixExpr::MemberAccessExpr>) {
//       return {
//         {"name", "MemberAccessExpr"},
//         {"pos", this->pos.GetMetaData()},
//         {"data", {
//           {"lhs", expr.lhs->GetMetaData()},
//           {"member", expr.member}
//         }}
//       };
//     } else if constexpr (std::is_same_v<T, PostfixExpr::MacroExpr>) {
//       return {
//         {"name", "MacroExpr"},
//         {"pos", this->pos.GetMetaData()},
//         {"data", expr.qualifying_name}
//       };
//     } else {
//       static_assert(always_false<T>::value, "non-exhaustive vistor!");
//     }
//   }, this->expr);
// }

json PowerExpr::GetMetaData() const {
  if (exprs.size() == 1) {
    // fallthrough
    return exprs[0]->GetMetaData();
  } else {
    return {
      {"name", "PowerExpr"},
      {"pos", this->pos.GetMetaData()},
      {"data", GetJsonForVec(exprs)}
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
      {"data", {
        {"rhs", rhs->GetMetaData()},
        {"ops", data}
      }}
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
      {"data", {
        {"lhs", lhs->GetMetaData()},
        {"ops", data}
      }}
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
      {"data", {
        {"lhs", lhs->GetMetaData()},
        {"ops", data}
      }}
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
      {"data", {
        {"lhs", lhs->GetMetaData()},
        {"ops", data}
      }}
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
      {"data", GetJsonForVec(exprs)}
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
      {"data", GetJsonForVec(exprs)}
    };
  }
}

// json Block::GetMetaData() const {
//   json data;
//   for (auto &expr: exprs) data.push_back(expr->GetMetaData());
//   return {
//     {"name", "Block"},
//     {"pos", this->pos.GetMetaData()},
//     {"data", data}
//   };
// }

// json Expr::GetMetaData() const {
//   return std::visit([this](auto &&expr)->json {
//     using T = std::decay_t<decltype(expr)>;
//     constexpr bool is_func_def = std::is_same_v<T, Expr::FuncBlock>;
//     if constexpr (is_any<T, std::unique_ptr<ConditionalExpr>,
//                 std::unique_ptr<WhileBlock>, std::unique_ptr<ForBlock>,
//                 std::unique_ptr<IfBlock>>) {
//       return expr->GetMetaData();
//     } else if constexpr (is_func_def || std::is_same_v<T, Expr::TupleBlock>) {
//       return {
//         {"name", is_func_def ? "FuncBlock" : "TupleBlock"},
//         {"pos", this->pos.GetMetaData()},
//         {"data", {
//           {is_func_def ? "func" : "tuple_def", expr.def->GetMetaData()},
//           {"block", expr.block->GetMetaData()}
//         }}
//       };
//     } else {
//       static_assert(always_false<T>::value, "non-exhaustive vistor!");
//     }
//   }, this->expr);
// }

// json WhileBlock::GetMetaData() const {
//   json else_block_data = this->else_block ? this->else_block.value()->GetMetaData(): "null";
//   return {
//     {"name", "While"},
//     {"pos", this->pos.GetMetaData()},
//     {"cond", this->expr->GetMetaData()},
//     {"block", this->block->GetMetaData()},
//     {"else", else_block_data}
//   };
// }

// json ForBlock::GetMetaData() const {
//   json else_block_data = this->else_block ? this->else_block.value()->GetMetaData(): "null";
//   return {
//     {"name", "For"},
//     {"pos", this->pos.GetMetaData()},
//     {"contents", this->expr.GetMetaData()},
//     {"block", this->block->GetMetaData()},
//     {"else", else_block_data}
//   };
// }

// json ForBlockContents::GetMetaData() const {
//   return std::visit([this](auto &&expr)->json {
//     using T = std::decay_t<decltype(expr)>;
//     if constexpr (std::is_same_v<T, ForBlockContents::ForIn>) {
//       return {
//         {"name", "ForIn"},
//         {"pos", this->pos.GetMetaData()},
//         {"data", {
//           {"lhs", expr.lhs},
//           {"rhs", expr.rhs->GetMetaData()}
//         }}
//       };
//     } else if constexpr (std::is_same_v<T, ForBlockContents::ForTraditional>) {
//       json data;
//       if (expr.start) data["start"] = expr.start.value()->GetMetaData();
//       if (expr.stop) data["stop"] = expr.stop.value()->GetMetaData();
//       if (expr.step) data["step"] = expr.step.value()->GetMetaData();
//       return {
//         {"name", "LogicalOrExpr"},
//         {"pos", this->pos.GetMetaData()},
//         {"data", data}
//       };
//     } else {
//       static_assert(always_false<T>::value, "non-exhaustive vistor!");
//     }
//   }, this->expr);
// }

// json IfBlock::GetMetaData() const {
//   json else_block_data = this->else_block
//     ? this->else_block.value()->GetMetaData()
//     : "null";
//   return {
//     {"name", "If"},
//     {"pos", this->pos.GetMetaData()},
//     {"cond", this->cond->GetMetaData()},
//     {"block", this->block->GetMetaData()},
//     {"else", else_block_data}
//   };
// }

// json ElseBlock::GetMetaData() const {
//   return std::visit([](auto &&expr)->json {
//     return expr->GetMetaData();
//   }, this->expr);
// }

// json TupleMember::GetMetaData() const {
//   return {
//     {"name", "TupleMember"},
//     {"pos", this->pos.GetMetaData()},
//     {"id", id},
//     {"type", (type ? type.value()->GetMetaData() : "null")}
//   };
// }

// json TupleDefinition::GetMetaData() const {
//   json data;
//   for (auto &member: members) {
//     data.push_back(member->GetMetaData());
//   }
//   return data;
// }

// bool TypeMember::IsMap() const {
//   return std::holds_alternative<ArrayOrMapType>(expr)
//     ? std::get<ArrayOrMapType>(expr).second_type.has_value()
//     : false;
// }

// json TypeMember::GetMetaData() const {
//   return std::visit([this](auto &&expr)->json {
//     using T = std::decay_t<decltype(expr)>;
//     if constexpr (is_any<T, std::unique_ptr<FuncCall>,
//                             std::unique_ptr<FuncDefinition>,
//                             std::unique_ptr<TupleDefinition>>) {
//       return expr->GetMetaData();
//     } else if constexpr (std::is_same_v<T, std::string>) {
//       return expr;
//     } else if constexpr (std::is_same_v<T, TypeMember::ArrayOrMapType>) {
//       json data;
//       if (IsMap()) {
//         data["key_type"] = expr.first_type->GetMetaData();
//         data["value_type"] = (*expr.second_type)->GetMetaData();
//       } else {
//         data["value_type"] = expr.first_type->GetMetaData();
//       }
//       return {
//         {"name", IsMap() ? "MapType" : "ArrayType"},
//         {"pos", this->pos.GetMetaData()},
//         {"data", data}
//       };
//     } else {
//       static_assert(always_false<T>::value, "non-exhaustive vistor!");
//     }
//   }, this->expr);
// }

// json TypeExpr::GetMetaData() const {
//   return std::visit([this](auto &&expr)->json {
//     using T = std::decay_t<decltype(expr)>;
//     if constexpr (std::is_same_v<T, std::unique_ptr<TypeMember>>) {
//       return expr->GetMetaData();
//     } else if constexpr (std::is_same_v<T, TypeExpr::ComplexExpr>) {
//       json data = {{"expr", expr.lhs->GetMetaData()}};
//       if (expr.or_expr) data["or_expr"] = expr.or_expr.value()->GetMetaData();
//       if (expr.implements_expr) data["implements_expr"] = expr.implements_expr.value()->GetMetaData();

//       return {
//         {"name", "TypeExpr"},
//         {"pos", this->pos.GetMetaData()},
//         {"data", data}
//       };
//     } else {
//       static_assert(always_false<T>::value, "non-exhaustive vistor!");
//     }
//   }, this->expr);
// }

// json FuncDefinition::GetMetaData() const {
//   json data;
//   if (args) data["args"] = args.value()->GetMetaData();
//   if (ret_type) data["ret_type"] = ret_type.value()->GetMetaData();

//   return {
//     {"name", "FuncDefinition"},
//     {"pos", this->pos.GetMetaData()},
//     {"data", data}
//   };
// }

// json Constant::GetMetaData() const {
//   return std::visit([this](auto &&expr)->json {
//     using T = std::decay_t<decltype(expr)>;
//     if constexpr (is_any<T, std::unique_ptr<ArrayExpr>, std::unique_ptr<MapExpr>>) {
//       return expr->GetMetaData();
//     } else if constexpr (is_any<T, double, i64, std::string, char>) {
//       return expr;
//     } else {
//       static_assert(always_false<T>::value, "non-exhaustive vistor!");
//     }
//   }, this->data);
// }

// json MapExpr::GetMetaData() const {
//   Assert(keys.size() == values.size(), "keys and value size must match", "key size: ", keys.size(), "value size: ", values.size());
//   json data;
//   for (size_t i = 0; i < keys.size(); i++) {
//     data["key"] = keys[i]->GetMetaData();
//     data["value"] = values[i]->GetMetaData();
//   }
//   return data;
// }

// json ArrayExpr::GetMetaData() const {
//   json data;
//   for (auto &val: values) {
//     data.push_back(val->GetMetaData());
//   }
//   return data;
// }

// }
