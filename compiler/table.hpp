/*
  Represents effectively variables and relevant identifiers
*/

#ifndef TABLE_HPP
#define TABLE_HPP

#include "ast.hpp"

class Table {
 private:


 public:
  void RegisterDeclaration(const std::unique_ptr<VarDecl> &decl);
  std::optional<Declaration> CheckDeclaration(const LineStr &name) const;


};

#endif