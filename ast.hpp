#ifndef AST_HPP
#define AST_HPP

#include "id_cache.hpp"
#include <cstdint>
#include <limits>
#include <unordered_map>
#include <vector>
#include <deque>

using AstNodeIndex = std::uint32_t;
static const AstNodeIndex UndefinedAstNodeIndex = std::numeric_limits<AstNodeIndex>::max();

class OrderedDict {
public:
  OrderedDict() = default;
  OrderedDict(const OrderedDict&) = delete;
  OrderedDict(OrderedDict&&) = delete;
  OrderedDict& operator=(const OrderedDict&) = delete;
  OrderedDict& operator=(OrderedDict&&) = delete;

  void append(IdIndex name, AstNodeIndex node_index) {
    auto it = m_map.find(name);
    if (it == m_map.end()) {
      m_map.emplace(name, node_index);
      m_nodes.emplace_back(node_index);
    }
  }

  void append(AstNodeIndex node_index) {
    m_nodes.emplace_back(node_index);
  }

  AstNodeIndex find(IdIndex name) const {
    auto it = m_map.find(name);

    if (it == m_map.end()) {
      return UndefinedAstNodeIndex;
    }
    return it->second;
  }

  const std::vector<AstNodeIndex>& get_nodes() const { return m_nodes;}

private:
  using Map = std::unordered_map<IdIndex, AstNodeIndex> ;
  Map m_map;
  std::vector<AstNodeIndex> m_nodes;
};

struct AstNode {
  enum class Kind {
    None, Type, Value, 
    I8Type, I16Type, I32Type, U8Type, U16Type, U32Type, F32Type, F64Type, 
    StructType, UnionType,
    FunType, FunTypeWithNamedParams, LocalVariable, GlobalVariable, StringLiteral, 
    I8Literal, I16Literal, I32Literal, U8Literal, U16Literal, U32Literal, 
    F32Literal, F64Literal,
    AssignExpr, EqualExpr, GreatExpr, GreatOrEqualExpr, LessExpr, LessOrEqualExpr, 
    ParenthExpr, NegExpr,
    StructField, UnionField,
    Function, Struct, Union, BlockScope,
    VariableDeclStmt, BlockStmt, FunctionDeclStmt, StructDeclStmt, UnionDeclStmt, IfElseStmt, WhileStmt,
  };
  enum class StatementKind {};

  AstNode(Kind kind) {
    memset((void*)this, 0, sizeof(AstNode));
    this->kind = kind;
  }
  AstNode(const AstNode&) = delete;
  AstNode(AstNode&&) = delete;
  AstNode& operator=(const AstNode&) = delete;
  AstNode& operator=(AstNode&&) = delete;

  bool is_stmt() const { return is_stmt(kind); }

  static bool is_stmt(AstNode::Kind kind) {
    switch (kind) {
      case AstNode::Kind::VariableDeclStmt:
      case AstNode::Kind::BlockStmt:
      case AstNode::Kind::FunctionDeclStmt:
      case AstNode::Kind::StructDeclStmt:
      case AstNode::Kind::UnionDeclStmt:
      case AstNode::Kind::IfElseStmt:
      case AstNode::Kind::WhileStmt:
        return true;
      default:
        return false;
    }
  }

  bool is_expr() const { return is_expr(kind); }

  static bool is_expr(AstNode::Kind kind) {
    switch (kind) {
      case AstNode::Kind::AssignExpr:
      case AstNode::Kind::EqualExpr:
      case AstNode::Kind::GreatExpr:
      case AstNode::Kind::GreatOrEqualExpr:
      case AstNode::Kind::LessExpr:
      case AstNode::Kind::LessOrEqualExpr:
        return true;
      default:
        return false;
    }
  }

  bool is_scope() const { return is_scope(kind); }

  static bool is_scope(AstNode::Kind kind) {
    switch (kind) {
      case AstNode::Kind::Function:
      case AstNode::Kind::Struct:
      case AstNode::Kind::Union:
        return true;
      default:
        return false;
    }
  }

  bool is_type() const { return is_type(kind); }

  static bool is_type(AstNode::Kind kind) {
    switch (kind) {
      case AstNode::Kind::I8Type: 
      case AstNode::Kind::I16Type: 
      case AstNode::Kind::I32Type: 
      case AstNode::Kind::U8Type: 
      case AstNode::Kind::U16Type: 
      case AstNode::Kind::U32Type: 
      case AstNode::Kind::F32Type:
      case AstNode::Kind::F64Type:
      case AstNode::Kind::StructType:
      case AstNode::Kind::UnionType:
      case AstNode::Kind::FunType:
        return true;
      default: 
        return false;
    }
  } 

  bool is_value() const { return is_value(kind); }

  static bool is_value(AstNode::Kind kind) {
    switch (kind) {
      case AstNode::Kind::LocalVariable:
      case AstNode::Kind::GlobalVariable:
      case AstNode::Kind::StringLiteral:
      case AstNode::Kind::I8Literal:
      case AstNode::Kind::I16Literal:
      case AstNode::Kind::I32Literal:
      case AstNode::Kind::U8Literal:
      case AstNode::Kind::U16Literal:
      case AstNode::Kind::U32Literal:
      case AstNode::Kind::F32Literal:
      case AstNode::Kind::F64Literal:
      case AstNode::Kind::StructField:
      case AstNode::Kind::UnionField:
        return true;
      default:
        return false;
    }
  }

  ~AstNode() { clean(); }

  void clean() {
    switch (kind) {
      case AstNode::Kind::FunTypeWithNamedParams:
        delete node.fun_type_with_named_params.names;
      case AstNode::Kind::FunType:
        delete node.fun_type.param_types;
        break;
      case AstNode::Kind::Function:
      case AstNode::Kind::Struct:
      case AstNode::Kind::Union:
        delete node.scope.dict;
        break;
      case AstNode::Kind::BlockStmt:
        delete node.block_stmt.stmts;
        break;
      default:
        break;
    }
    memset((void*)this, 0, sizeof(AstNode));
    kind = AstNode::Kind::None;
  }

  Kind kind;

  union Node {
    struct Value {
      AstNodeIndex type; 
    };

    Value value;

    struct {
      Value value;
      IdIndex name;
    } global_variable;

    struct {
      Value value;
      IdIndex name;
    } local_variable;

    struct UnaryExpr {
      AstNodeIndex expr;
    };

    UnaryExpr parenth_expr;
    UnaryExpr neg_expr;

    struct BinaryExpr {
      AstNodeIndex left;
      AstNodeIndex right;
    };

    BinaryExpr assign_expr;
    BinaryExpr equal_expr;
    BinaryExpr great_expr;
    BinaryExpr great__or_equal_expr;
    BinaryExpr less_expr;
    BinaryExpr less_or_equal_expr;

    struct StringLiteral {
      Value value;
      IdIndex string;
    };

    template <typename Type>
    struct NumberLiteral {
      Value value;
      Type literal_value;
    };

    StringLiteral string_literal;
    NumberLiteral<int8_t> i8_literal;
    NumberLiteral<int16_t> i16_literal;
    NumberLiteral<int32_t> i32_literal;
    NumberLiteral<int64_t> i64_literal;
    NumberLiteral<uint8_t> u8_literal;
    NumberLiteral<uint16_t> u16_literal;
    NumberLiteral<uint32_t> u32_literal;

    struct FunType {
      AstNodeIndex return_type;
      std::vector<AstNodeIndex>* param_types;
      IdIndex name;

      void add_param_type(AstNodeIndex type) {
        if (!param_types) param_types = new std::vector<AstNodeIndex>;
        param_types->emplace_back(type);
      }
    };
    struct FunTypeWithNamedParams {
      FunType fun_type;
      std::vector<IdIndex>* names;

      void add_name(IdIndex id) {
        if (!names) names = new std::vector<IdIndex>;
        names->emplace_back(id);
      }
    };
    FunType fun_type;
    FunTypeWithNamedParams fun_type_with_named_params;

    struct {
      AstNodeIndex struct_scope;
    } struct_type;

    struct {
      Value value;
      IdIndex name;
      uint32_t offset;
    } struct_field;

    struct Scope {
      AstNodeIndex outer_scope;
      IdIndex name;
      OrderedDict* dict;

      void add_node(AstNodeIndex node_idx, IdIndex name = UndefinedIdIndex) {
        if (!dict) dict = new OrderedDict;
        if (name != UndefinedIdIndex) {
          dict->append(name, node_idx);
        } else {
          dict->append(node_idx);
        }
      }
    };

    Scope scope;

    struct {
      Scope scope;
      AstNodeIndex block_stmt;
    } block_scope;

    struct {
      Scope scope;
      AstNodeIndex function_type_with_named_params;
    } function;

    struct StructOrUnion {
      Scope scope;
    };

    StructOrUnion struc;
    StructOrUnion unio;

    struct {
      AstNodeIndex variable;
      AstNodeIndex init_expr;
    } variable_decl_stmt;

    struct {
      AstNodeIndex block_scope;
      std::vector<AstNodeIndex>* stmts;

      void add_stmt(AstNodeIndex node_idx) {
        if (!stmts) stmts = new std::vector<AstNodeIndex>();
        stmts->emplace_back(node_idx);
      }
    } block_stmt;

    struct {
      AstNodeIndex expr;
      AstNodeIndex stmt;
      AstNodeIndex else_stmt;
    } if_else_stmt;

    struct {
      AstNodeIndex expr;
      AstNodeIndex stmt;
    } while_stmt;

  };
  Node node;
};

class Ast {
public:
  AstNode& get_node(AstNodeIndex index) {
    return nodes[index];
  }

  AstNodeIndex create(AstNode::Kind kind) {
    if (removed.empty()) {
      nodes.emplace_back(kind);
      return nodes.size() - 1;
    }
    const auto index = removed.back();
    nodes[index].kind = kind;
    removed.pop_back();
    return index;
  }

  void remove(AstNodeIndex index) {
    nodes[index].clean();
    removed.emplace_back(index);
  }
private:
  std::deque<AstNode> nodes;
  std::deque<AstNodeIndex> removed;
};

#endif  // AST_HPP
