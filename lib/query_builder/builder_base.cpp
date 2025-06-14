#include <query_builder/builder_base.h>
#include <relation/relation_manager.h>

namespace NOrm::NRelation::Builder {

////////////////////////////////////////////////////////////////////////////////
// TString implementation

std::string TString::Build(TBuilderBasePtr builder) const {
    return builder->BuildString(Value_);
}

EClauseType TString::Type() const {
    return EClauseType::String;
}

////////////////////////////////////////////////////////////////////////////////
// TInt implementation

std::string TInt::Build(TBuilderBasePtr builder) const {
    return builder->BuildInt(Value_);
}

EClauseType TInt::Type() const {
    return EClauseType::Int;
}

////////////////////////////////////////////////////////////////////////////////
// TFloat implementation

std::string TFloat::Build(TBuilderBasePtr builder) const {
    return builder->BuildFloat(Value_);
}

EClauseType TFloat::Type() const {
    return EClauseType::Float;
}

////////////////////////////////////////////////////////////////////////////////
// TBool implementation

std::string TBool::Build(TBuilderBasePtr builder) const {
    return builder->BuildBool(Value_);
}

EClauseType TBool::Type() const {
    return EClauseType::Bool;
}

////////////////////////////////////////////////////////////////////////////////
// TExpression implementation

std::string TExpression::Build(TBuilderBasePtr builder) const {
    return builder->BuildExpression(ExpressionType_, Operands_);
}

EClauseType TExpression::Type() const {
    return EClauseType::Expression;
}

////////////////////////////////////////////////////////////////////////////////
// TAll implementation

std::string TAll::Build(TBuilderBasePtr builder) const {
    return builder->BuildAll();
}

EClauseType TAll::Type() const {
    return EClauseType::All;
}

////////////////////////////////////////////////////////////////////////////////
// TColumn implementation

std::string TColumn::Build(TBuilderBasePtr builder) const {
    return builder->BuildColumn(FieldPath_, ColumnType_);
}

EClauseType TColumn::Type() const {
    return EClauseType::Column;
}

////////////////////////////////////////////////////////////////////////////////
// TDefault implementation

std::string TTable::Build(TBuilderBasePtr builder) const {
    return builder->BuildTable(Path_);
}

EClauseType TTable::Type() const {
    return EClauseType::Table;
}

////////////////////////////////////////////////////////////////////////////////
// TDefault implementation

std::string TDefault::Build(TBuilderBasePtr builder) const {
    return builder->BuildDefault();
}

EClauseType TDefault::Type() const {
    return EClauseType::Default;
}

////////////////////////////////////////////////////////////////////////////////

std::string TJoin::Build(TBuilderBasePtr builder) const {
    return builder->BuildJoin(Table_, Condition_, JoinType_);
}

EClauseType TJoin::Type() const {
    return EClauseType::Join;
}

////////////////////////////////////////////////////////////////////////////////
// TSelect implementation

std::string TSelect::Build(TBuilderBasePtr builder) const {
    return builder->BuildSelect(Selectors_, From_, Join_, Where_, GroupBy_, Having_, OrderBy_, Limit_);
}

EClauseType TSelect::Type() const {
    return EClauseType::Select;
}

////////////////////////////////////////////////////////////////////////////////
// TInsert implementation

std::string TInsert::Build(TBuilderBasePtr builder) const {
    return builder->BuildInsert(Table_, Selectors_, IsValues_, Values_, IsDoUpdate_, DoUpdate_);
}

EClauseType TInsert::Type() const {
    return EClauseType::Insert;
}

////////////////////////////////////////////////////////////////////////////////
// TUpdate implementation

std::string TUpdate::Build(TBuilderBasePtr builder) const {
    return builder->BuildUpdate(Table_, Updates_, Where_);
}

EClauseType TUpdate::Type() const {
    return EClauseType::Update;
}

////////////////////////////////////////////////////////////////////////////////
// TDelete implementation

std::string TDelete::Build(TBuilderBasePtr builder) const {
    return builder->BuildDelete(Table_, Where_);
}

EClauseType TDelete::Type() const {
    return EClauseType::Delete;
}

////////////////////////////////////////////////////////////////////////////////
// TTruncate implementation

std::string TTruncate::Build(TBuilderBasePtr builder) const {
    return builder->BuildTruncate(Path_);
}

EClauseType TTruncate::Type() const {
    return EClauseType::Truncate;
}

////////////////////////////////////////////////////////////////////////////////
// TStartTransaction implementation

std::string TStartTransaction::Build(TBuilderBasePtr builder) const {
    return builder->BuildStartTransaction(read_only);
}

EClauseType TStartTransaction::Type() const {
    return EClauseType::StartTransaction;
}

////////////////////////////////////////////////////////////////////////////////
// TCommitTransaction implementation

std::string TCommitTransaction::Build(TBuilderBasePtr builder) const {
    return builder->BuildCommitTransaction();
}

EClauseType TCommitTransaction::Type() const {
    return EClauseType::CommitTransaction;
}

////////////////////////////////////////////////////////////////////////////////
// TRollbackTransaction implementation

std::string TRollbackTransaction::Build(TBuilderBasePtr builder) const {
    return builder->BuildRollbackTransaction();
}

EClauseType TRollbackTransaction::Type() const {
    return EClauseType::RollbackTransaction;
}

////////////////////////////////////////////////////////////////////////////////
// TCreateTable implementation

std::string TCreateTable::Build(TBuilderBasePtr builder) const {
    auto& tableInfo = TRelationManager::GetInstance().GetParentTable(Message_->GetPath());
    return builder->BuildCreateTable(tableInfo);
}

EClauseType TCreateTable::Type() const {
    return EClauseType::CreateTable;
}

////////////////////////////////////////////////////////////////////////////////
// TDropTable implementation

std::string TDropTable::Build(TBuilderBasePtr builder) const {
    auto& tableInfo = TRelationManager::GetInstance().GetParentTable(Message_->GetPath());
    return builder->BuildDropTable(tableInfo);
}

EClauseType TDropTable::Type() const {
    return EClauseType::DropTable;
}

////////////////////////////////////////////////////////////////////////////////
// TAddColumn implementation

std::string TAddColumn::Build(TBuilderBasePtr builder) const {
    return builder->BuildAddColumn(Field_);
}

EClauseType TAddColumn::Type() const {
    return EClauseType::CreateColumn;
}

void TAddColumn::SetColumn(NOrm::NRelation::TPrimitiveFieldInfoPtr field) {
    Field_ = field;
}

////////////////////////////////////////////////////////////////////////////////
// TDropColumn implementation

std::string TDropColumn::Build(TBuilderBasePtr builder) const {
    return builder->BuildDropColumn(Field_);
}

EClauseType TDropColumn::Type() const {
    return EClauseType::DropColumn;
}

void TDropColumn::SetColumn(NOrm::NRelation::TPrimitiveFieldInfoPtr field) {
    Field_ = field;
}

////////////////////////////////////////////////////////////////////////////////
// TAlterColumn implementation

std::string TAlterColumn::Build(TBuilderBasePtr builder) const {
    return builder->BuildAlterColumn(New_, Old_);
}

EClauseType TAlterColumn::Type() const {
    return EClauseType::AlterColumn;
}

void TAlterColumn::SetNew(NOrm::NRelation::TPrimitiveFieldInfoPtr newField) {
    New_ = newField;
}

void TAlterColumn::SetOld(NOrm::NRelation::TPrimitiveFieldInfoPtr oldField) {
    Old_ = oldField;
}

////////////////////////////////////////////////////////////////////////////////
// TAlterTable implementation

std::string TAlterTable::Build(TBuilderBasePtr builder) const {
    if (!Operations_.empty()) {
        for (const auto& op : Operations_) {
            if (auto addCol = std::dynamic_pointer_cast<TAddColumn>(op)) {
                if (auto field = addCol->GetField()) {
                    auto& tableInfo = TRelationManager::GetInstance().GetParentTable(field->GetPath());
                    return builder->BuildAlterTable(tableInfo, Operations_);
                }
            } else if (auto dropCol = std::dynamic_pointer_cast<TDropColumn>(op)) {
                if (auto field = dropCol->GetField()) {
                    auto& tableInfo = TRelationManager::GetInstance().GetParentTable(field->GetPath());
                    return builder->BuildAlterTable(tableInfo, Operations_);
                }
            } else if (auto alterCol = std::dynamic_pointer_cast<TAlterColumn>(op)) {
                if (auto field = alterCol->GetNew()) {
                    auto& tableInfo = TRelationManager::GetInstance().GetParentTable(field->GetPath());
                    return builder->BuildAlterTable(tableInfo, Operations_);
                }
            }
        }
    }
    
    // Если не удалось определить таблицу, бросаем исключение
    THROW("Cannot determine table for ALTER TABLE operation");
}

EClauseType TAlterTable::Type() const {
    return EClauseType::AlterTable;
}

void TAlterTable::AddOperation(TClausePtr operation) {
    Operations_.push_back(operation);
}

////////////////////////////////////////////////////////////////////////////////
// TQuery implementation

std::string TQuery::Build(TBuilderBasePtr builder) const {
    std::vector<std::string> results;
    
    for (auto clause : Clauses_) {
        results.push_back(clause->Build(builder));
    }
    
    return builder->JoinQueries(results);
}

} // namespace NOrm::NRelation::Builder
