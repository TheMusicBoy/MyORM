#include <query_builder/builder_base.h>
#include <relation/relation_manager.h>

namespace NOrm::NRelation::Builder {

// Реализация функции BuildClause - единая точка входа для построения SQL
////////////////////////////////////////////////////////////////////////////////
// TString implementation

EClauseType TString::Type() const {
    return EClauseType::String;
}

////////////////////////////////////////////////////////////////////////////////
// TInt implementation

EClauseType TInt::Type() const {
    return EClauseType::Int;
}

////////////////////////////////////////////////////////////////////////////////
// TFloat implementation

EClauseType TFloat::Type() const {
    return EClauseType::Float;
}

////////////////////////////////////////////////////////////////////////////////
// TBool implementation

EClauseType TBool::Type() const {
    return EClauseType::Bool;
}

////////////////////////////////////////////////////////////////////////////////
// TExpression implementation

EClauseType TExpression::Type() const {
    return EClauseType::Expression;
}

////////////////////////////////////////////////////////////////////////////////
// TAll implementation

EClauseType TAll::Type() const {
    return EClauseType::All;
}

////////////////////////////////////////////////////////////////////////////////
// TColumn implementation

EClauseType TColumn::Type() const {
    return EClauseType::Column;
}

////////////////////////////////////////////////////////////////////////////////
// TDefault implementation

EClauseType TTable::Type() const {
    return EClauseType::Table;
}

////////////////////////////////////////////////////////////////////////////////
// TDefault implementation

EClauseType TDefault::Type() const {
    return EClauseType::Default;
}

////////////////////////////////////////////////////////////////////////////////

EClauseType TJoin::Type() const {
    return EClauseType::Join;
}

////////////////////////////////////////////////////////////////////////////////
// TSelect implementation

EClauseType TSelect::Type() const {
    return EClauseType::Select;
}

////////////////////////////////////////////////////////////////////////////////
// TInsert implementation

EClauseType TInsert::Type() const {
    return EClauseType::Insert;
}

////////////////////////////////////////////////////////////////////////////////
// TUpdate implementation

EClauseType TUpdate::Type() const {
    return EClauseType::Update;
}

////////////////////////////////////////////////////////////////////////////////
// TDelete implementation

EClauseType TDelete::Type() const {
    return EClauseType::Delete;
}

////////////////////////////////////////////////////////////////////////////////
// TTruncate implementation

EClauseType TTruncate::Type() const {
    return EClauseType::Truncate;
}

////////////////////////////////////////////////////////////////////////////////
// TStartTransaction implementation

EClauseType TStartTransaction::Type() const {
    return EClauseType::StartTransaction;
}

////////////////////////////////////////////////////////////////////////////////
// TCommitTransaction implementation

EClauseType TCommitTransaction::Type() const {
    return EClauseType::CommitTransaction;
}

////////////////////////////////////////////////////////////////////////////////
// TRollbackTransaction implementation

EClauseType TRollbackTransaction::Type() const {
    return EClauseType::RollbackTransaction;
}

////////////////////////////////////////////////////////////////////////////////
// TCreateTable implementation

EClauseType TCreateTable::Type() const {
    return EClauseType::CreateTable;
}

////////////////////////////////////////////////////////////////////////////////
// TDropTable implementation

EClauseType TDropTable::Type() const {
    return EClauseType::DropTable;
}

////////////////////////////////////////////////////////////////////////////////
// TAddColumn implementation

EClauseType TAddColumn::Type() const {
    return EClauseType::CreateColumn;
}

void TAddColumn::SetColumn(NOrm::NRelation::TPrimitiveFieldInfoPtr field) {
    Field_ = field;
}

////////////////////////////////////////////////////////////////////////////////
// TDropColumn implementation

EClauseType TDropColumn::Type() const {
    return EClauseType::DropColumn;
}

void TDropColumn::SetColumn(NOrm::NRelation::TPrimitiveFieldInfoPtr field) {
    Field_ = field;
}

////////////////////////////////////////////////////////////////////////////////
// TAlterColumn implementation

EClauseType TAlterColumn::Type() const {
    return EClauseType::AlterColumn;
}

////////////////////////////////////////////////////////////////////////////////
// TAlterTable implementation

EClauseType TAlterTable::Type() const {
    return EClauseType::AlterTable;
}

void TAlterTable::AddOperation(TClausePtr operation) {
    Operations_.push_back(operation);
}

////////////////////////////////////////////////////////////////////////////////

std::string TBuilderBase::BuildClause(TClausePtr clause) {
    if (!clause) {
        return "";
    }

    switch (clause->Type()) {
        case EClauseType::String:
            return this->BuildString(std::static_pointer_cast<TString>(clause));
            
        case EClauseType::Int:
            return this->BuildInt(std::static_pointer_cast<TInt>(clause));
            
        case EClauseType::Float:
            return this->BuildFloat(std::static_pointer_cast<TFloat>(clause));
            
        case EClauseType::Bool:
            return this->BuildBool(std::static_pointer_cast<TBool>(clause));
            
        case EClauseType::Expression:
            return this->BuildExpression(std::static_pointer_cast<TExpression>(clause));
            
        case EClauseType::All:
            return this->BuildAll(std::static_pointer_cast<TAll>(clause));
            
        case EClauseType::Column:
            return this->BuildColumn(std::static_pointer_cast<TColumn>(clause));
            
        case EClauseType::Table:
            return this->BuildTable(std::static_pointer_cast<TTable>(clause));
            
        case EClauseType::Default:
            return this->BuildDefault(std::static_pointer_cast<TDefault>(clause));
            
        case EClauseType::Join:
            return this->BuildJoin(std::static_pointer_cast<TJoin>(clause));
            
        case EClauseType::Select:
            return this->BuildSelect(std::static_pointer_cast<TSelect>(clause));
            
        case EClauseType::Insert:
            return this->BuildInsert(std::static_pointer_cast<TInsert>(clause));
            
        case EClauseType::Update:
            return this->BuildUpdate(std::static_pointer_cast<TUpdate>(clause));
            
        case EClauseType::Delete:
            return this->BuildDelete(std::static_pointer_cast<TDelete>(clause));
            
        case EClauseType::Truncate:
            return this->BuildTruncate(std::static_pointer_cast<TTruncate>(clause));
            
        case EClauseType::StartTransaction:
            return this->BuildStartTransaction(std::static_pointer_cast<TStartTransaction>(clause));
            
        case EClauseType::CommitTransaction:
            return this->BuildCommitTransaction(std::static_pointer_cast<TCommitTransaction>(clause));
            
        case EClauseType::RollbackTransaction:
            return this->BuildRollbackTransaction(std::static_pointer_cast<TRollbackTransaction>(clause));
            
        case EClauseType::CreateTable:
            return this->BuildCreateTable(std::static_pointer_cast<TCreateTable>(clause));
            
        case EClauseType::DropTable:
            return this->BuildDropTable(std::static_pointer_cast<TDropTable>(clause));
            
        case EClauseType::AlterTable:
            return this->BuildAlterTable(std::static_pointer_cast<TAlterTable>(clause));
            
        case EClauseType::CreateColumn:
            return this->BuildAddColumn(std::static_pointer_cast<TAddColumn>(clause));
            
        case EClauseType::DropColumn:
            return this->BuildDropColumn(std::static_pointer_cast<TDropColumn>(clause));
            
        case EClauseType::AlterColumn:
            return this->BuildAlterColumn(std::static_pointer_cast<TAlterColumn>(clause));
            
        default:
            THROW("Uknown type of clause: {}", static_cast<int>(clause->Type()));
    }
    
    return "";
}


} // namespace NOrm::NRelation::Builder

