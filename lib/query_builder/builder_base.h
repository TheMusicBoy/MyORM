#pragma once

#include <lib/relation/proto/query.pb.h>
#include <memory>
#include <relation/field.h>
#include <relation/message.h>
#include <relation/relation_manager.h>
#include <string>
#include <vector>

namespace NOrm::NRelation::Builder {

////////////////////////////////////////////////////////////////////////////////

class TBuilderBase;
using TBuilderBasePtr = std::shared_ptr<TBuilderBase>;

enum class EClauseType {
    String,
    Int,
    Float,
    Bool,
    Expression,
    All,
    Column,
    Table,
    Default,
    Join,
    Select,
    Insert,
    Update,
    Delete,
    Truncate,
    CreateTable,
    AlterTable,
    DropTable,

    StartTransaction,
    CommitTransaction,
    RollbackTransaction,

    CreateColumn,
    DropColumn,
    AlterColumn
};

////////////////////////////////////////////////////////////////////////////////

class TClause {
  public:
    virtual ~TClause() = default;
    virtual EClauseType Type() const = 0;

  private:
    friend TBuilderBase;
};

using TClausePtr = std::shared_ptr<TClause>;

////////////////////////////////////////////////////////////////////////////////
// Базовые типы данных

class TString : public TClause {
  public:
    TString(const std::string& value = "")
        : Value_(value) {}

    EClauseType Type() const override;

    const std::string& GetValue() const {
        return Value_;
    }
    void SetValue(const std::string& value) {
        Value_ = value;
    }

  private:
    std::string Value_;
    friend TBuilderBase;
};

using TStringPtr = std::shared_ptr<TString>;

class TInt : public TClause {
  public:
    TInt(int32_t value = 0)
        : Value_(value) {}

    EClauseType Type() const override;

    int32_t GetValue() const {
        return Value_;
    }
    void SetValue(int32_t value) {
        Value_ = value;
    }

  private:
    int32_t Value_;
    friend TBuilderBase;
};

using TIntPtr = std::shared_ptr<TInt>;

class TFloat : public TClause {
  public:
    TFloat(double value = 0.0)
        : Value_(value) {}

    EClauseType Type() const override;

    double GetValue() const {
        return Value_;
    }
    void SetValue(double value) {
        Value_ = value;
    }

  private:
    double Value_;
    friend TBuilderBase;
};

using TFloatPtr = std::shared_ptr<TFloat>;

class TBool : public TClause {
  public:
    TBool(bool value = false)
        : Value_(value) {}

    EClauseType Type() const override;

    bool GetValue() const {
        return Value_;
    }
    void SetValue(bool value) {
        Value_ = value;
    }

  private:
    bool Value_;
    friend TBuilderBase;
};

using TBoolPtr = std::shared_ptr<TBool>;

class TExpression : public TClause {
  public:
    TExpression(NQuery::EExpressionType expressionType = NQuery::EExpressionType::equals, const std::vector<TClausePtr>& operands = {})
        : ExpressionType_(expressionType),
          Operands_(operands) {}

    EClauseType Type() const override;

    const std::vector<TClausePtr>& GetOperands() const {
        return Operands_;
    }
    void SetOperands(const std::vector<TClausePtr>& operands) {
        Operands_ = operands;
    }
    NQuery::EExpressionType GetExpressionType() const {
        return ExpressionType_;
    }
    void SetExpressionType(NQuery::EExpressionType type) {
        ExpressionType_ = type;
    }

  private:
    std::vector<TClausePtr> Operands_;
    NQuery::EExpressionType ExpressionType_;
    friend TBuilderBase;
};

using TExpressionPtr = std::shared_ptr<TExpression>;

class TAll : public TClause {
  public:
    TAll() = default;

    EClauseType Type() const override;
};

using TAllPtr = std::shared_ptr<TAll>;

class TDefault : public TClause {
  public:
    TDefault() = default;

    EClauseType Type() const override;
};

using TDefaultPtr = std::shared_ptr<TDefault>;

enum EKeyType {
    Simple = 0,
    Primary = 1,
    Index = 2
};

class TColumn : public TClause {
  public:
    TColumn(const std::vector<uint32_t>& tablePath, const std::vector<uint32_t> fieldPath)
        : TablePath_(tablePath), FieldPath_(fieldPath) {}

    EClauseType Type() const override;

    void SetPath(const std::vector<uint32_t>& table, const std::vector<uint32_t>& path) {
        TablePath_ = table;
        FieldPath_ = path;
    }
    const std::vector<uint32_t>& GetFieldPath() const {
        return FieldPath_;
    }
    const std::vector<uint32_t>& GetTablePath() const {
        return TablePath_;
    }
    EKeyType GetKeyType() const {
        return KeyType_;
    }
    void SetKeyType(EKeyType type) {
        KeyType_ = type;
    }
    NQuery::EColumnType GetColumnType() const {
        return ColumnType_;
    }
    void SetColumnType(NQuery::EColumnType type) {
        ColumnType_ = type;
    }

  private:
    std::vector<uint32_t> TablePath_;
    std::vector<uint32_t> FieldPath_;
    EKeyType KeyType_;
    NQuery::EColumnType ColumnType_;
    friend TBuilderBase;
};

using TColumnPtr = std::shared_ptr<TColumn>;

class TColumnDefinition : public TClause {
  public:
    TColumnDefinition(const std::vector<uint32_t>& tablePath, const std::vector<uint32_t> fieldPath)
        : TablePath_(tablePath), FieldPath_(fieldPath) {}

    EClauseType Type() const override;

    void SetPath(const std::vector<uint32_t>& table, const std::vector<uint32_t>& path) {
        TablePath_ = table;
        FieldPath_ = path;
    }

    const std::vector<uint32_t>& GetFieldPath() const {
        return FieldPath_;
    }
    const std::vector<uint32_t>& GetTablePath() const {
        return TablePath_;
    }
    EKeyType GetKeyType() const {
        return KeyType_;
    }
    void SetKeyType(EKeyType type) {  // Исправлено с GetKeyType на SetKeyType
        KeyType_ = type;
    }

    // Методы для TypeInfo_
    const TValueInfo& GetTypeInfo() const {
        return TypeInfo_;
    }
    void SetTypeInfo(const TValueInfo& typeInfo) {
        TypeInfo_ = typeInfo;
    }

    // Методы для DefaultValueString_
    const std::string& GetDefaultValueString() const {
        return DefaultValueString_;
    }
    void SetDefaultValueString(const std::string& defaultValueString) {
        DefaultValueString_ = defaultValueString;
    }

    // Методы для HasDefault_
    bool HasDefault() const {
        return HasDefault_;
    }
    void SetHasDefault(bool hasDefault) {
        HasDefault_ = hasDefault;
    }

    // Методы для Unique_
    bool IsUnique() const {
        return Unique_;
    }
    void SetUnique(bool unique) {
        Unique_ = unique;
    }

    // Методы для IsRequired_
    bool IsRequired() const {
        return IsRequired_;
    }
    void SetIsRequired(bool isRequired) {
        IsRequired_ = isRequired;
    }

    // Методы для IsPrimaryKey_
    bool IsPrimaryKey() const {
        return IsPrimaryKey_;
    }
    void SetIsPrimaryKey(bool isPrimaryKey) {
        IsPrimaryKey_ = isPrimaryKey;
    }

    // Методы для AutoIncrement_
    bool IsAutoIncrement() const {
        return AutoIncrement_;
    }
    void SetAutoIncrement(bool autoIncrement) {
        AutoIncrement_ = autoIncrement;
    }

  private:
    std::vector<uint32_t> TablePath_;
    std::vector<uint32_t> FieldPath_;

    EKeyType KeyType_;

    TValueInfo TypeInfo_;
    std::string DefaultValueString_;
    bool HasDefault_;
    bool Unique_;
    bool IsRequired_;
    bool IsPrimaryKey_;
    bool AutoIncrement_;

    friend TBuilderBase;
};

using TColumnDefinitionPtr = std::shared_ptr<TColumnDefinition>;

class TTable : public TClause {
  public:
    TTable(const TMessagePath& path = TMessagePath()) : Path_(path) {}

    EClauseType Type() const override;

    const TMessagePath& GetPath() const {
        return Path_;
    }
    void SetPath(const TMessagePath& path) {
        Path_ = path;
    }

  private:
    TMessagePath Path_;
    friend TBuilderBase;
};

using TTablePtr = std::shared_ptr<TTable>;

class TJoin : public TClause {
  public:
    enum EJoinType { Left, Inner, ExclusiveLeft };

    TJoin(TMessagePath table, TClausePtr condition, EJoinType type)
        : Table_(table),
          Condition_(condition),
          JoinType_(type) {}

    EClauseType Type() const override;

    void SetTable(const TMessagePath& table) { Table_ = table; }
    TMessagePath GetTable() const { return Table_; }
    void SetCondition(const TClausePtr& condition) { Condition_ = condition; }
    TClausePtr GetCondition() const { return Condition_; }
    void SetJoinType(EJoinType type) { JoinType_ = type; }
    EJoinType GetJoinType() const { return JoinType_; }

  private:
    TMessagePath Table_;
    TClausePtr Condition_;
    EJoinType JoinType_;
};

using TJoinPtr = std::shared_ptr<TJoin>;

class TSelect : public TClause {
  public:
    TSelect(
        const std::vector<TClausePtr>& selectors = {},
        const std::vector<TClausePtr>& from = {},
        const std::vector<TClausePtr>& join = {},
        TClausePtr where = nullptr,
        TClausePtr groupBy = nullptr,
        TClausePtr having = nullptr,
        TClausePtr orderBy = nullptr,
        TClausePtr limit = nullptr
    )
        : Selectors_(selectors),
          From_(from),
          Where_(where),
          GroupBy_(groupBy),
          Having_(having),
          OrderBy_(orderBy),
          Limit_(limit) {}

    EClauseType Type() const override;

    const std::vector<TClausePtr>& GetSelectors() const {
        return Selectors_;
    }
    void SetSelectors(const std::vector<TClausePtr>& selectors) {
        Selectors_ = selectors;
    }
    const std::vector<TClausePtr>& GetFrom() const {
        return From_;
    }
    void SetFrom(const std::vector<TClausePtr>& from) {
        From_ = from;
    }
    const std::vector<TClausePtr>& GetJoin() const {
        return Join_;
    }
    void SetJoin(const std::vector<TClausePtr>& join) {
        Join_ = join;
    }
    TClausePtr GetWhere() const {
        return Where_;
    }
    void SetWhere(TClausePtr where) {
        Where_ = where;
    }
    TClausePtr GetGroupBy() const {
        return GroupBy_;
    }
    void SetGroupBy(TClausePtr groupBy) {
        GroupBy_ = groupBy;
    }
    TClausePtr GetHaving() const {
        return Having_;
    }
    void SetHaving(TClausePtr having) {
        Having_ = having;
    }
    TClausePtr GetOrderBy() const {
        return OrderBy_;
    }
    void SetOrderBy(TClausePtr orderBy) {
        OrderBy_ = orderBy;
    }
    TClausePtr GetLimit() const {
        return Limit_;
    }
    void SetLimit(TClausePtr limit) {
        Limit_ = limit;
    }

  private:
    std::vector<TClausePtr> Selectors_;
    std::vector<TClausePtr> From_;
    std::vector<TClausePtr> Join_;
    TClausePtr Where_;
    TClausePtr GroupBy_;
    TClausePtr Having_;
    TClausePtr OrderBy_;
    TClausePtr Limit_;
    friend TBuilderBase;
};

using TSelectPtr = std::shared_ptr<TSelect>;

class TInsert : public TClause {
  public:
    TInsert(
        const TMessagePath& table,
        const std::vector<TClausePtr>& selectors = {},
        bool isValues = false,
        const std::vector<std::vector<TClausePtr>>& values = {},
        bool isDoUpdate = false,
        const std::vector<std::pair<TClausePtr, TClausePtr>>& doUpdate = {}
    )
        : Table_(table),
          Selectors_(selectors),
          IsValues_(isValues),
          Values_(values),
          IsDoUpdate_(isDoUpdate),
          DoUpdate_(doUpdate) {}

    const TMessagePath& GetTable() const {
        return Table_;
    }

    EClauseType Type() const override;

    const std::vector<TClausePtr>& GetSelectors() const {
        return Selectors_;
    }
    void SetSelectors(const std::vector<TClausePtr>& selectors) {
        Selectors_ = selectors;
    }
    bool GetIsValues() const {
        return IsValues_;
    }
    void SetIsValues(bool isValues) {
        IsValues_ = isValues;
    }
    const std::vector<std::vector<TClausePtr>>& GetValues() const {
        return Values_;
    }
    void SetValues(const std::vector<std::vector<TClausePtr>>& values) {
        Values_ = values;
    }
    bool GetIsDoUpdate() const {
        return IsDoUpdate_;
    }
    void SetIsDoUpdate(bool isDoUpdate) {
        IsDoUpdate_ = isDoUpdate;
    }
    const std::vector<std::pair<TClausePtr, TClausePtr>>& GetDoUpdate() const {
        return DoUpdate_;
    }
    void SetDoUpdate(const std::vector<std::pair<TClausePtr, TClausePtr>>& doUpdate) {
        DoUpdate_ = doUpdate;
    }

  private:
    TMessagePath Table_;
    std::vector<TClausePtr> Selectors_;
    bool IsValues_;
    std::vector<std::vector<TClausePtr>> Values_;

    bool IsDoUpdate_;
    std::vector<std::pair<TClausePtr, TClausePtr>> DoUpdate_;

    friend TBuilderBase;
};

using TInsertPtr = std::shared_ptr<TInsert>;

class TUpdate : public TClause {
  public:
    TUpdate(const TMessagePath& table, const std::vector<std::pair<TClausePtr, TClausePtr>>& updates = {}, TClausePtr where = nullptr)
        : Table_(table),
          Updates_(updates),
          Where_(where) {}

    const TMessagePath& GetTable() const {
        return Table_;
    }

    EClauseType Type() const override;

    const std::vector<std::pair<TClausePtr, TClausePtr>>& GetUpdates() const {
        return Updates_;
    }
    void SetUpdates(const std::vector<std::pair<TClausePtr, TClausePtr>>& updates) {
        Updates_ = updates;
    }
    TClausePtr GetWhere() const {
        return Where_;
    }
    void SetWhere(TClausePtr where) {
        Where_ = where;
    }

  private:
    TMessagePath Table_;
    std::vector<std::pair<TClausePtr, TClausePtr>> Updates_;
    TClausePtr Where_;
    friend TBuilderBase;
};

using TUpdatePtr = std::shared_ptr<TUpdate>;

class TDelete : public TClause {
  public:
    TDelete(const TMessagePath& table, TClausePtr where = nullptr)
        : Table_(table), Where_(where) {}

    const TMessagePath& GetTable() const {
        return Table_;
    }

    EClauseType Type() const override;

    TClausePtr GetWhere() const {
        return Where_;
    }
    void SetWhere(TClausePtr where) {
        Where_ = where;
    }

  private:
    TMessagePath Table_;
    TClausePtr Where_;

    friend TBuilderBase;
};

using TDeletePtr = std::shared_ptr<TDelete>;

class TTruncate : public TClause {
  public:
    TTruncate(const TMessagePath& path = TMessagePath())
        : Path_(path) {}

    EClauseType Type() const override;

    const TMessagePath& GetPath() const {
        return Path_;
    }
    void SetPath(const TMessagePath& path) {
        Path_ = path;
    }

  private:
    TMessagePath Path_;
    friend TBuilderBase;
};

using TTruncatePtr = std::shared_ptr<TTruncate>;

class TStartTransaction : public TClause {
  public:
    TStartTransaction(bool readOnly = false)
        : read_only(readOnly) {}

    EClauseType Type() const override;

    bool GetReadOnly() const {
        return read_only;
    }
    void SetReadOnly(bool readOnly) {
        read_only = readOnly;
    }

  private:
    bool read_only;
    friend TBuilderBase;
};

using TStartTransactionPtr = std::shared_ptr<TStartTransaction>;

class TCommitTransaction : public TClause {
  public:
    TCommitTransaction() = default;
    EClauseType Type() const override;
};

using TCommitTransactionPtr = std::shared_ptr<TCommitTransaction>;

class TRollbackTransaction : public TClause {
  public:
    TRollbackTransaction() = default;
    EClauseType Type() const override;
};

using TRollbackTransactionPtr = std::shared_ptr<TRollbackTransaction>;

class TCreateTable : public TClause {
  public:
    TCreateTable(NOrm::NRelation::TMessageInfoPtr message = nullptr)
        : Message_(message) {}

    EClauseType Type() const override;

    NOrm::NRelation::TMessageInfoPtr GetMessage() const {
        return Message_;
    }
    void SetMessage(NOrm::NRelation::TMessageInfoPtr message) {
        Message_ = message;
    }

  private:
    NOrm::NRelation::TMessageInfoPtr Message_;
    friend TBuilderBase;
};

using TCreateTablePtr = std::shared_ptr<TCreateTable>;

class TDropTable : public TClause {
  public:
    TDropTable(NOrm::NRelation::TMessageInfoPtr message = nullptr)
        : Message_(message) {}

    EClauseType Type() const override;

    NOrm::NRelation::TMessageInfoPtr GetMessage() const {
        return Message_;
    }
    void SetMessage(NOrm::NRelation::TMessageInfoPtr message) {
        Message_ = message;
    }

  private:
    NOrm::NRelation::TMessageInfoPtr Message_;
    friend TBuilderBase;
};

using TDropTablePtr = std::shared_ptr<TDropTable>;

class TAddColumn : public TClause {
  public:
    TAddColumn(NOrm::NRelation::TPrimitiveFieldInfoPtr field = nullptr)
        : Field_(field) {}

    EClauseType Type() const override;

    void SetColumn(NOrm::NRelation::TPrimitiveFieldInfoPtr field);

    NOrm::NRelation::TPrimitiveFieldInfoPtr GetField() const {
        return Field_;
    }

  private:
    NOrm::NRelation::TPrimitiveFieldInfoPtr Field_;
    friend TBuilderBase;
};

using TAddColumnPtr = std::shared_ptr<TAddColumn>;

class TDropColumn : public TClause {
  public:
    TDropColumn(NOrm::NRelation::TPrimitiveFieldInfoPtr field = nullptr)
        : Field_(field) {}

    EClauseType Type() const override;

    void SetColumn(NOrm::NRelation::TPrimitiveFieldInfoPtr field);

    NOrm::NRelation::TPrimitiveFieldInfoPtr GetField() const {
        return Field_;
    }

  private:
    NOrm::NRelation::TPrimitiveFieldInfoPtr Field_;
    friend TBuilderBase;
};

using TDropColumnPtr = std::shared_ptr<TDropColumn>;

class TAlterColumn : public TClause {
  public:
    TAlterColumn(TColumnPtr column) : Column_(column) {}

    EClauseType Type() const override;

    TColumnPtr GetColumn() {
        return Column_;
    }

    enum EAlterType {
        kNone = 0,
        kSetDefault = 1,
        kDropDefault = 2,
        kSetRequired = 3,
        kDropRequired = 4,
        kSetType = 5
    };

    void DropRequired() { AlterType_ = EAlterType::kDropRequired; }
    void SetRequired() { AlterType_ = EAlterType::kSetRequired; }
    void DropDefault() { AlterType_ = EAlterType::kDropDefault; }
    void SetDefault(const TValueInfo* info) {
        AlterType_ = EAlterType::kDropDefault;
        TypeInfo_ = info;
    }
    void SetType(const TValueInfo* info) {
        AlterType_ = EAlterType::kSetType;
        TypeInfo_ = info;
    }

    TAlterColumn::EAlterType GetAlterType() const {
        return AlterType_;
    }

    const TValueInfo* GetValueType() const {
        return TypeInfo_;
    }

  private:
    TColumnPtr Column_;
    EAlterType AlterType_;
    const TValueInfo* TypeInfo_;

    friend TBuilderBase;
};

using TAlterColumnPtr = std::shared_ptr<TAlterColumn>;

class TAlterTable : public TClause {
  public:
    TAlterTable(const std::vector<TClausePtr>& operations = {})
        : Operations_(operations) {}

    EClauseType Type() const override;

    void AddOperation(TClausePtr operation);

    const std::vector<TClausePtr>& GetOperations() const {
        return Operations_;
    }
    void SetOperations(const std::vector<TClausePtr>& operations) {
        Operations_ = operations;
    }

  private:
    std::vector<TClausePtr> Operations_;
    friend TBuilderBase;
};

using TAlterTablePtr = std::shared_ptr<TAlterTable>;

class TQuery {
  public:
    TQuery(const std::vector<TClausePtr>& clauses = {})
        : Clauses_(clauses) {}

    const std::vector<TClausePtr>& GetClauses() const {
        return Clauses_;
    }
    void SetClauses(const std::vector<TClausePtr>& clauses) {
        Clauses_ = clauses;
    }
    void AddClause(TClausePtr clause) {
        Clauses_.push_back(clause);
    }

  private:
    std::vector<TClausePtr> Clauses_;
    friend TBuilderBase;
};

using TQueryPtr = std::shared_ptr<TQuery>;

////////////////////////////////////////////////////////////////////////////////

class TBuilderBase {
  public:
    virtual ~TBuilderBase() = default;

    std::string BuildClause(TClausePtr clause);

  protected:
    virtual std::string BuildString(TStringPtr value) = 0;
    virtual std::string BuildInt(TIntPtr value) = 0;
    virtual std::string BuildFloat(TFloatPtr value) = 0;
    virtual std::string BuildBool(TBoolPtr value) = 0;

    virtual std::string BuildExpression(TExpressionPtr expression) = 0;

    virtual std::string BuildAll(TAllPtr all) = 0;

    virtual std::string BuildColumn(TColumnPtr column) = 0;
    virtual std::string BuildTable(TTablePtr table) = 0;

    virtual std::string BuildDefault(TDefaultPtr defaultVal) = 0;

    virtual std::string BuildSelect(TSelectPtr select) = 0;

    virtual std::string BuildJoin(TJoinPtr join) = 0;

    virtual std::string BuildInsert(TInsertPtr insert) = 0;

    virtual std::string BuildUpdate(TUpdatePtr update) = 0;

    virtual std::string BuildDelete(TDeletePtr deleteClause) = 0;
    virtual std::string BuildTruncate(TTruncatePtr truncate) = 0;

    virtual std::string BuildStartTransaction(TStartTransactionPtr startTransaction) = 0;
    virtual std::string BuildCommitTransaction(TCommitTransactionPtr commitTransaction) = 0;
    virtual std::string BuildRollbackTransaction(TRollbackTransactionPtr rollbackTransaction) = 0;

    virtual std::string BuildColumnDefinition(TColumnDefinitionPtr columnDefinition) = 0;

    virtual std::string BuildCreateTable(TCreateTablePtr createTable) = 0;
    virtual std::string BuildDropTable(TDropTablePtr dropTable) = 0;
    virtual std::string BuildAlterTable(TAlterTablePtr alterTable) = 0;

    virtual std::string BuildAddColumn(TAddColumnPtr addColumn) = 0;
    virtual std::string BuildDropColumn(TDropColumnPtr dropColumn) = 0;
    virtual std::string BuildAlterColumn(TAlterColumnPtr alterColumn) = 0;

    virtual std::string JoinQueries(const std::vector<std::string>& queries) = 0;
};

class TBuilderFabricBase {
  public:
    virtual TBuilderBasePtr NewBuilder() const;
};

using TBuilderFabricPtr = std::shared_ptr<TBuilderFabricBase>;

} // namespace NOrm::NRelation::Builder
