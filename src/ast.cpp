#include "ast.hpp"

#include <type_traits>

const char *AssignmentOpToStr(AssignmentOp op) {
    switch (op) {
        case AssignmentOp::AdditionEqual: return "+=";
        case AssignmentOp::SubtractionEqual: return "-=";
        case AssignmentOp::Equal: return "=";
        case AssignmentOp::DivisionEqual: return "/=";
        case AssignmentOp::PowerEqual: return "**=";
        case AssignmentOp::ModulusEqual: return "%=";
        case AssignmentOp::MultiplyEqual: return "*=";
        case AssignmentOp::IntDivisionEqual: return "%/=";
        default: throw __FILE__":AssignmentOpToStr() Unhandled case";
    }
}

const char *PostfixOpToStr(PostfixOp op) {
    switch (op) {
        case PostfixOp::Increment: return "++";
        case PostfixOp::Decrement: return "--";
        default: throw __FILE__":PostfixOpToStr() Unhandled case";
    }
}

const char *PrefixOpToStr(PrefixOp op) {
    switch (op) {
        case PrefixOp::Increment: return "++";
        case PrefixOp::Decrement: return "--";
        case PrefixOp::Negate: return "!";
        case PrefixOp::Negative: return "-";
        case PrefixOp::Positive: return "+";
        default: throw __FILE__":PrefixOpToStr() Unhandled case";
    }
}

const char *MultiplicativeOpToStr(MultiplicativeOp op) {
    switch (op) {
        case MultiplicativeOp::Multiplication: return "*";
        case MultiplicativeOp::Division: return "/";
        case MultiplicativeOp::Modulus: return "%";
        case MultiplicativeOp::IntDivision: return "%/";
        default: throw __FILE__":MultiplicativeOpToStr() Unhandled case";
    }
}

const char *AdditiveOpToStr(AdditiveOp op) {
    switch (op) {
        case AdditiveOp::Addition: return "+";
        case AdditiveOp::Subtraction: return "-";
        default: throw __FILE__":AdditiveOpToStr() Unhandled case";
    }
}

const char *RelationalOpToStr(RelationalOp op) {
    switch (op) {
        case RelationalOp::GreaterThan: return ">";
        case RelationalOp::LessThan: return "<";
        case RelationalOp::GreaterThanEqual: return ">=";
        case RelationalOp::LessThanEqual: return "<=";
        default: throw __FILE__":RelationalOpToStr() Unhandled case";
    }
}

const char *EqualityOpToStr(EqualityOp op) {
    switch (op) {
        case EqualityOp::Equal: return "==";
        case EqualityOp::NotEqual: return "!=";
        default: throw __FILE__":EqualityOpToStr() Unhandled case";
    }
}

std::ostream &operator <<(std::ostream &out, const LineRange &pos) {
    // 0:1 -> 1:2
    out << pos.line_start << ":" << pos.col_start << " -> " << pos.line_end << ":" << pos.col_end;
    return out;
}

json LineRange::GetMetaData() const {
    return {
        { this->line_start, this->col_start },
        { this->line_end, this->col_end }
    };
}

json FileLevelExpression::GetMetaData() const {
    std::vector<json> meta_data;
    for (auto &expr : this->expressions) {
        meta_data.push_back(expr->GetMetaData());
    }
    return {
        {"name", "FileLevelExpression"},
        {"pos", this->pos.GetMetaData()},
        {"data", meta_data}
    };
}

json TopLevelExpression::GetMetaData() const {
    json data = std::visit([](auto &value)->json { return value.GetMetaData(); }, this->expr);

    if (!this->ret) return data;

    // state we return since this->ret == true
    data.emplace("ret", true);
    return {
        {"name", "TopLevelExpression"},
        {"pos", this->pos.GetMetaData()},
        {"data", data}
    };
}

json PrimaryExpression::GetMetaData() const {
    if (auto str = std::get_if<std::string>(&this->expr)) {
        return *str;
    }

    // else just return the json stuff for the sub value
    // no need to present primary stuff
    return std::visit([](auto &value)->json { return value->GetMetaData(); }, this->expr);
}

json AssignmentExpression::GetMetaData() const {
    json data = json::array();
    char *name;
    if (auto declare_var = std::get_if<AssignmentExpression::DeclareVariable>(&this->expr)) {
        data.emplace_back(std::get<0>(*declare_var)->GetMetaData());
        if (std::get<1>(*declare_var)) data.emplace_back(std::get<1>(*declare_var).value->GetMetaData());
        name = "AssignmentExpression.DeclareVariable";
    } else if (auto std_assignment = std::get_if<AssignmentExpression::StandardAssignment>(&this->expr)) {
        data.emplace_back("lhs", std::get<0>(*std_assignment)->GetMetaData());
        data.emplace_back("op", AssignmentOpToStr(std::get<1>(*std_assignment)));
        data.emplace_back("rhs", std::get<2>(*std_assignment)->GetMetaData());
        name = "AssignmentExpression.StandardAssignment";
    } else {
        throw __FILE__":AssignmentExpression::GetMetaData() cases not exhaustive";
    }

    return {
        {"name", name},
        {"pos", this->pos.GetMetaData()},
        {"data", data}
    };
}

json FuncCall::GetMetaData() const {
    json data = json::array();
    data.emplace("func", this->func->GetMetaData());
    data.emplace("args", json::array());
    for (auto &expr: this->args) {
        data["args"].emplace(expr->GetMetaData());
    }

    return {
        {"name", "FuncCall"},
        {"pos", this->pos.GetMetaData()},
        {"data", data}
    };
}

json PostfixExpression::GetMetaData() const {
    json data;
    if (auto primary_expr = std::get_if<std::unique_ptr<PrimaryExpression>>(&this->expr)) {
        data = (*primary_expr)->GetMetaData();
    } else if (auto func_call = std::get_if<std::unique_ptr<FuncCall>>(&this->expr)) {
        data = (*func_call)->GetMetaData();
    } else {
        data = std::visit([](auto &&expr)->json {
            using T = std::decay_t<decltype(expr)>;
            if constexpr (std::is_same_v<T, PostfixExpression::IndexExpr>) return {
                {"name", "IndexExpr"},
                {"pos", this->pos.GetMetaData()},
                {"data", {
                    { "lhs", std::get<0>(expr)->GetMetaData() },
                    { "index", std::get<1>(expr)->GetMetaData() }
                }}
            };
            if constexpr (std::is_same_v<T, PostfixExpression::SliceExpr>) return {
                {"name", "SliceExpr"},
                {"pos", this->pos.GetMetaData()},
                {"data", {
                    { "lhs", std::get<0>(expr)->GetMetaData() },
                    { "start", std::get<1>(expr) ? (json)nullptr : (json)std::get<1>(expr).value->GetMetaValue() },
                    { "stop", std::get<2>(expr) ? (json)nullptr : (json)std::get<2>(expr).value->GetMetaValue() },
                    { "step", std::get<3>(expr) ? (json)nullptr : (json)std::get<3>(expr).value->GetMetaValue() },
                }}
            };
            if constexpr (std::is_same_v<T, PostfixExpression::FoldExpr>) return {
                {"name", "FoldExpr"},
                {"pos", this->pos.GetMetaData()},
                {"data", {
                    { "func", std::get<0>(expr)->GetMetaData() },
                    { "fold_expr", std::get<1>(expr)->GetMetaData() }
                }}
            };
            if constexpr (std::is_same_v<T, PostfixExpression::PostfixOpExpr>) return {
                {"name", "PostfixOpExpr"},
                {"pos", this->pos.GetMetaData()},
                {"data", {
                    { "expr", std::get<0>(expr)->GetMetaData() },
                    { "op", PostfixOpToStr(std::get<1>(expr)) }
                }}
            }
            if constexpr (std::is_same_v<T, PostfixExpression::MemberAccessExpr>) return {
                {"name", "MemberAccessExpr"},
                {"pos", this->pos.GetMetaData()},
                {"data", {
                    { "lhs", std::get<0>(expr)->GetMetaData() },
                    { "member", std::get<1>(expr) }
                }}
            }
            if constexpr (std::is_same_v<T, PostfixExpression::MacroExpr>) return {
                {"name", "MacroExpr"},
                {"pos", this->pos.GetMetaData()},
                {"data", {
                    { "lhs", std::get<0>(expr) },
                    { "members", std::get<1>(expr) }
                }}
            }
        }, this->expr);
    }
    return data;
}
