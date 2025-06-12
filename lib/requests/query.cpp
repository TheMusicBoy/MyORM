#include <requests/query.h>

namespace NOrm::NRelation {

namespace {

////////////////////////////////////////////////////////////////////////////////

template <typename T>
TClause RegisterCluase(const NApi::TQuery& input, uint32_t startPoint) {
    T value;
    value.FromProto(input, startPoint);
    return value;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace

// Helper function to create clause from proto
TClause CreateClauseFromProto(const NApi::TQuery& input, uint32_t startPoint) {
    switch (input.clauses().at(startPoint).value_case()) {
        case NOrm::NApi::TClause::ValueCase::kString:
            return RegisterCluase<TString>(input, startPoint);
        case NOrm::NApi::TClause::ValueCase::kInteger:
            return RegisterCluase<TInt>(input, startPoint);
        case NOrm::NApi::TClause::ValueCase::kFloat:
            return RegisterCluase<TFloat>(input, startPoint);
        case NOrm::NApi::TClause::ValueCase::kBool:
            return RegisterCluase<TBool>(input, startPoint);
        case NOrm::NApi::TClause::ValueCase::kExpression: 
            return RegisterCluase<TExpression>(input, startPoint);
        case NOrm::NApi::TClause::ValueCase::kColumn:
            return RegisterCluase<TColumn>(input, startPoint);
        case NOrm::NApi::TClause::ValueCase::kSelect:
            return RegisterCluase<TSelect>(input, startPoint);
        case NOrm::NApi::TClause::ValueCase::kValueRows:
            return RegisterCluase<TValueRows>(input, startPoint);
        case NOrm::NApi::TClause::ValueCase::kDefaultValues:
            return RegisterCluase<TDefaultValues>(input, startPoint);
        case NOrm::NApi::TClause::ValueCase::kDoNothing:
            return RegisterCluase<TDoNothing>(input, startPoint);
        case NOrm::NApi::TClause::ValueCase::kDoUpdate:
            return RegisterCluase<TDoUpdate>(input, startPoint);
        case NOrm::NApi::TClause::ValueCase::kInsert:
            return RegisterCluase<TInsert>(input, startPoint);
        case NOrm::NApi::TClause::ValueCase::kUpdate:
            return RegisterCluase<TUpdate>(input, startPoint);
        case NOrm::NApi::TClause::ValueCase::kDelete:
            return RegisterCluase<TDelete>(input, startPoint);
        case NOrm::NApi::TClause::ValueCase::kTruncate:
            return RegisterCluase<TTruncate>(input, startPoint);
        case NOrm::NApi::TClause::ValueCase::kStartTransaction:
            return RegisterCluase<TStartTransaction>(input, startPoint);
        case NOrm::NApi::TClause::ValueCase::kCommitTransaction:
            return RegisterCluase<TCommitTransaction>(input, startPoint);
        case NOrm::NApi::TClause::ValueCase::kRollbackTransaction:
            return RegisterCluase<TRollbackTransaction>(input, startPoint);
        default: {
            throw std::runtime_error("Unknown type of expression");
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// TString implementation

void TStringImpl::ToProto(NApi::TQuery* output) const {
    auto string = new NApi::TString();
    string->set_value(Value_);
    output->add_clauses()->set_allocated_string(string);
}

void TStringImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    Value_ = input.clauses().at(startPoint).string().value();
}

TString& TString::SetValue(const std::string& value) {
    std::dynamic_pointer_cast<TStringImpl>(Impl_)->Value_ = value;
    return *this;
}

std::string TString::GetValue() const {
    return std::dynamic_pointer_cast<TStringImpl>(Impl_)->Value_;
}

////////////////////////////////////////////////////////////////////////////////
// TInt implementation

void TIntImpl::ToProto(NApi::TQuery* output) const {
    auto integer = new NApi::TInt();
    integer->set_value(Value_);
    output->add_clauses()->set_allocated_integer(integer);
}

void TIntImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    Value_ = input.clauses().at(startPoint).integer().value();
}

TInt& TInt::SetValue(int32_t value) {
    std::dynamic_pointer_cast<TIntImpl>(Impl_)->Value_ = value;
    return *this;
}

int32_t TInt::GetValue() const {
    return std::dynamic_pointer_cast<TIntImpl>(Impl_)->Value_;
}

////////////////////////////////////////////////////////////////////////////////
// TFloat implementation

void TFloatImpl::ToProto(NApi::TQuery* output) const {
    auto float_val = new NApi::TFloat();
    float_val->set_value(Value_);
    output->add_clauses()->set_allocated_float_(float_val);
}

void TFloatImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    Value_ = input.clauses().at(startPoint).float_().value();
}

TFloat& TFloat::SetValue(double value) {
    std::dynamic_pointer_cast<TFloatImpl>(Impl_)->Value_ = value;
    return *this;
}

double TFloat::GetValue() const {
    return std::dynamic_pointer_cast<TFloatImpl>(Impl_)->Value_;
}

////////////////////////////////////////////////////////////////////////////////
// TBool implementation

void TBoolImpl::ToProto(NApi::TQuery* output) const {
    auto bool_val = new NApi::TBool();
    bool_val->set_value(Value_);
    output->add_clauses()->set_allocated_bool_(bool_val);
}

void TBoolImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    Value_ = input.clauses().at(startPoint).bool_().value();
}

TBool& TBool::SetValue(bool value) {
    std::dynamic_pointer_cast<TBoolImpl>(Impl_)->Value_ = value;
    return *this;
}

bool TBool::GetValue() const {
    return std::dynamic_pointer_cast<TBoolImpl>(Impl_)->Value_;
}

////////////////////////////////////////////////////////////////////////////////
// TExpression implementation

void TExpressionImpl::ToProto(NApi::TQuery* output) const {
    auto expressionVal = new NApi::TExpression();
    expressionVal->set_expression_type(ExpressionType_);

    for (const TClause& op : Operands_) {
        op.ToProto(output);
        expressionVal->add_operands(output->clauses_size() - 1);
    }

    output->add_clauses()->set_allocated_expression(expressionVal);
}

void TExpressionImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    ExpressionType_ = input.clauses().at(startPoint).expression().expression_type();
    for (const auto& op : input.clauses().at(startPoint).expression().operands()) {
        Operands_.emplace_back(CreateClauseFromProto(input, op));
    }
}

TExpression& TExpression::SetExpressionType(NQuery::EExpressionType type) {
    std::dynamic_pointer_cast<TExpressionImpl>(Impl_)->ExpressionType_ = type;
    return *this;
}

TExpression& TExpression::AddOperand(TClause operand) {
    std::dynamic_pointer_cast<TExpressionImpl>(Impl_)->Operands_.push_back(operand);
    return *this;
}

NQuery::EExpressionType TExpression::GetExpressionType() const {
    return std::dynamic_pointer_cast<TExpressionImpl>(Impl_)->ExpressionType_;
}

const std::vector<TClause>& TExpression::GetOperands() const {
    return std::dynamic_pointer_cast<TExpressionImpl>(Impl_)->Operands_;
}

////////////////////////////////////////////////////////////////////////////////
// TAll implementation

void TAllImpl::ToProto(NApi::TQuery* output) const {
    output->add_clauses()->mutable_all();
}

void TAllImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    // Nothing to do for TAll
}

void TAll::ToProto(NOrm::NApi::TQuery* output) const {
    Impl_->ToProto(output);
}

void TAll::FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) {
    Impl_->FromProto(input, startPoint);
}

////////////////////////////////////////////////////////////////////////////////
// TColumn implementation

void TColumnImpl::ToProto(NApi::TQuery* output) const {
    auto columnVal = new NApi::TColumn();
    for (const auto& num : Path_) {
        columnVal->add_field_path(num);
    }
    for (size_t pos = 0; pos < Path_.GetIndexSize(); pos++) {
        const auto& idx = Path_.GetIndex(pos);
        if (std::holds_alternative<TMessagePath::TAllIndex>(idx)) {
            NRelation::TAll().ToProto(output);
        } else if (std::holds_alternative<int64_t>(idx)) {
            NRelation::TInt().SetValue(std::get<int64_t>(idx)).ToProto(output);
        } else if (std::holds_alternative<double>(idx)) {
            NRelation::TFloat().SetValue(std::get<double>(idx)).ToProto(output);
        } else if (std::holds_alternative<std::string>(idx)) {
            NRelation::TString().SetValue(std::get<std::string>(idx)).ToProto(output);
        }
        columnVal->add_indexes(output->clauses_size() - 1);
    }
    columnVal->set_type(Type_);
    
    output->add_clauses()->set_allocated_column(columnVal);
}

void TColumnImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    const auto& column = input.clauses().at(startPoint).column();
    std::vector<TMessagePath::TIndex> indexes;
    for (const auto& idx : column.indexes()) {
        const auto& idxClause = input.clauses().at(idx);
        switch (idxClause.value_case()) {
            case NOrm::NApi::TClause::ValueCase::kAll:
                indexes.emplace_back(TMessagePath::TAllIndex());
                break;
            case NOrm::NApi::TClause::ValueCase::kInteger:
                indexes.emplace_back(idxClause.integer().value());
                break;
            case NOrm::NApi::TClause::ValueCase::kFloat:
                indexes.emplace_back(idxClause.float_().value());
                break;
            case NOrm::NApi::TClause::ValueCase::kString:
                indexes.emplace_back(idxClause.string().value());
                break;
            default:
                THROW("Uncapable type of index in path");
        }
    }
    Path_ = TMessagePath(column.field_path().begin(), column.field_path().end(), indexes.begin(), indexes.end());
    Type_ = column.type();
}

TColumn& TColumn::SetPath(const TMessagePath& path) {
    std::dynamic_pointer_cast<TColumnImpl>(Impl_)->Path_ = path;
    return *this;
}

TColumn& TColumn::SetType(NOrm::NQuery::EColumnType type) {
    std::dynamic_pointer_cast<TColumnImpl>(Impl_)->Type_ = type;
    return *this;
}

const TMessagePath& TColumn::GetPath() const {
    return std::dynamic_pointer_cast<TColumnImpl>(Impl_)->Path_;
}

NOrm::NQuery::EColumnType TColumn::GetType() const {
    return std::dynamic_pointer_cast<TColumnImpl>(Impl_)->Type_;
}

////////////////////////////////////////////////////////////////////////////////
// TDefault implementation

void TDefaultImpl::ToProto(NApi::TQuery* output) const {
    output->add_clauses()->mutable_default_();
}

void TDefaultImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    // Nothing to do for TDefault
}

void TDefault::ToProto(NOrm::NApi::TQuery* output) const {
    Impl_->ToProto(output);
}

void TDefault::FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) {
    Impl_->FromProto(input, startPoint);
}

////////////////////////////////////////////////////////////////////////////////
// TSelect implementation

void TSelectImpl::ToProto(NApi::TQuery* output) const {
    auto selectVal = new NApi::TSelect();
    
    for (auto& selector : Selectors_) {
        selector.ToProto(output);
        selectVal->add_selectors(output->clauses_size() - 1);
    }
    
    if (Where_) {
        Where_.ToProto(output);
        selectVal->set_where(output->clauses_size() - 1);
    }
    
    if (GroupBy_) {
        GroupBy_.ToProto(output);
        selectVal->set_group_by(output->clauses_size() - 1);
    }
    
    if (Having_) {
        Having_.ToProto(output);
        selectVal->set_having(output->clauses_size() - 1);
    }
    
    if (OrderBy_) {
        OrderBy_.ToProto(output);
        selectVal->set_order_by(output->clauses_size() - 1);
    }
    
    if (Limit_) {
        Limit_.ToProto(output);
        selectVal->set_limit(output->clauses_size() - 1);
    }
    
    output->add_clauses()->set_allocated_select(selectVal);
}

void TSelectImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    const auto& select = input.clauses().at(startPoint).select();
    
    Selectors_.clear();
    for (const auto& selector : select.selectors()) {
        Selectors_.emplace_back(CreateClauseFromProto(input, selector));
    }
    
    if (select.has_where()) {
        Where_ = CreateClauseFromProto(input, select.where());
    }
    
    if (select.has_group_by()) {
        GroupBy_ = CreateClauseFromProto(input, select.group_by());
    }
    
    if (select.has_having()) {
        Having_ = CreateClauseFromProto(input, select.having());
    }
    
    if (select.has_order_by()) {
        OrderBy_ = CreateClauseFromProto(input, select.order_by());
    }
    
    if (select.has_limit()) {
        Limit_ = CreateClauseFromProto(input, select.limit());
    }
}

TSelect& TSelect::Where(TClause conditions) {
    std::dynamic_pointer_cast<TSelectImpl>(Impl_)->Where_ = conditions;
    return *this;
}

TSelect& TSelect::GroupBy(TClause groupby) {
    std::dynamic_pointer_cast<TSelectImpl>(Impl_)->GroupBy_ = groupby;
    return *this;
}

TSelect& TSelect::Having(TClause having) {
    std::dynamic_pointer_cast<TSelectImpl>(Impl_)->Having_ = having;
    return *this;
}

TSelect& TSelect::OrderBy(TClause orderBy) {
    std::dynamic_pointer_cast<TSelectImpl>(Impl_)->OrderBy_ = orderBy;
    return *this;
}

TSelect& TSelect::Limit(TClause limit) {
    std::dynamic_pointer_cast<TSelectImpl>(Impl_)->Limit_ = limit;
    return *this;
}

////////////////////////////////////////////////////////////////////////////////
// TDefaultValueList implementation

void TDefaultValuesImpl::ToProto(NApi::TQuery* output) const {
    output->add_clauses()->mutable_default_values();
}

void TDefaultValuesImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    // Nothing to do for TDefaultValueList
}

void TDefaultValues::ToProto(NOrm::NApi::TQuery* output) const {
    Impl_->ToProto(output);
}

void TDefaultValues::FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) {
    Impl_->FromProto(input, startPoint);
}

////////////////////////////////////////////////////////////////////////////////
// TValueRows implementation

void TValueRowsImpl::ToProto(NApi::TQuery* output) const {
    auto rowValues = new NApi::TValueRows();
    
    for (auto& row : Rows_) {
        auto valueRow = rowValues->add_rows();
        for (auto& val : row) {
            val.ToProto(output);
            valueRow->add_values(output->clauses_size() - 1);
        }
    }
    
    output->add_clauses()->set_allocated_value_rows(rowValues);
}

void TValueRowsImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    const auto& rowValues = input.clauses().at(startPoint).value_rows();
    
    Rows_.clear();
    for (const auto& row : rowValues.rows()) {
        std::vector<TClause> rowValues;
        for (const auto& value : row.values()) {
            rowValues.emplace_back(CreateClauseFromProto(input, value));
        }
        Rows_.push_back(rowValues);
    }
}

TValueRows& TValueRows::AddRow(const std::vector<TClause>& row) {
    std::dynamic_pointer_cast<TValueRowsImpl>(Impl_)->Rows_.push_back(row);
    return *this;
}

const std::vector<std::vector<TClause>>& TValueRows::GetRows() const {
    return std::dynamic_pointer_cast<TValueRowsImpl>(Impl_)->Rows_;
}

////////////////////////////////////////////////////////////////////////////////
// TDoNothing implementation

void TDoNothingImpl::ToProto(NApi::TQuery* output) const {
    output->add_clauses()->mutable_do_nothing();
}

void TDoNothingImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    // Nothing to do for TDoNothing
}

void TDoNothing::ToProto(NOrm::NApi::TQuery* output) const {
    Impl_->ToProto(output);
}

void TDoNothing::FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) {
    Impl_->FromProto(input, startPoint);
}

////////////////////////////////////////////////////////////////////////////////
// TDoUpdate implementation

void TDoUpdateImpl::ToProto(NApi::TQuery* output) const {
    auto doUpdate = new NApi::TDoUpdate();
    
    for (auto& update : Updates_) {
        auto updateField = doUpdate->add_updates();
        update.first.ToProto(output);
        updateField->set_column_path(output->clauses_size() - 1);
        update.second.ToProto(output);
        updateField->set_expression(output->clauses_size() - 1);
    }
    
    output->add_clauses()->set_allocated_do_update(doUpdate);
}

void TDoUpdateImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    const auto& doUpdate = input.clauses().at(startPoint).do_update();

    Updates_.clear();
    for (const auto& update : doUpdate.updates()) {
        TColumn column = CreateClauseFromProto(input, update.column_path());
        auto expression = CreateClauseFromProto(input, update.expression());
        Updates_.emplace_back(std::make_pair(column, expression));
    }
}

TDoUpdate& TDoUpdate::AddUpdate(TColumn column, TClause expression) {
    std::dynamic_pointer_cast<TDoUpdateImpl>(Impl_)->Updates_.emplace_back(std::make_pair(column, expression));
    return *this;
}

const std::vector<std::pair<TColumn, TClause>>& TDoUpdate::GetUpdates() const {
    return std::dynamic_pointer_cast<TDoUpdateImpl>(Impl_)->Updates_;
}

////////////////////////////////////////////////////////////////////////////////
// TInsert implementation

void TInsertImpl::ToProto(NApi::TQuery* output) const {
    auto insert = new NApi::TInsert();
    
    for (auto& selector : Selectors_) {
        selector.ToProto(output);
        insert->add_selectors(output->clauses_size() - 1);
    }
    
    if (Values_) {
        Values_.ToProto(output);
        insert->set_values(output->clauses_size() - 1);
    }
    
    if (OnConflict_) {
        OnConflict_.ToProto(output);
        insert->set_on_conflict(output->clauses_size() - 1);
    }

    output->add_clauses()->set_allocated_insert(insert);
}

void TInsertImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    const auto& insert = input.clauses().at(startPoint).insert();
    
    Selectors_.clear();
    for (const auto& selector : insert.selectors()) {
        Selectors_.emplace_back(CreateClauseFromProto(input, selector));
    }
    
    if (insert.values() != 0) {
        Values_ = CreateClauseFromProto(input, insert.values());
    }
    
    if (insert.on_conflict() != 0) {
        OnConflict_ = CreateClauseFromProto(input, insert.on_conflict());
    }
}

TInsert& TInsert::Values(TClause values) {
    std::dynamic_pointer_cast<TInsertImpl>(Impl_)->Values_ = values;
    return *this;
}

TInsert& TInsert::Default() {
    std::dynamic_pointer_cast<TInsertImpl>(Impl_)->Values_ = TDefaultValues();
    return *this;
}

TInsert& TInsert::DoNothing() {
    std::dynamic_pointer_cast<TInsertImpl>(Impl_)->OnConflict_ = TDoNothing();
    return *this;
}

TInsert& TInsert::DoUpdate(TClause action) {
    std::dynamic_pointer_cast<TInsertImpl>(Impl_)->OnConflict_ = action;
    return *this;
}

TInsert& TInsert::OnConflict(TClause action) {
    std::dynamic_pointer_cast<TInsertImpl>(Impl_)->OnConflict_ = action;
    return *this;
}

////////////////////////////////////////////////////////////////////////////////
// TUpdate implementation

void TUpdateImpl::ToProto(NApi::TQuery* output) const {
    auto update = new NApi::TUpdate();
    
    for (auto& updateField : Updates_) {
        auto field = update->add_updates();
        updateField.first.ToProto(output);
        field->set_column_path(output->clauses_size() - 1);
        updateField.second.ToProto(output);
        field->set_expression(output->clauses_size() - 1);
    }
    
    if (Where_) {
        Where_.ToProto(output);
        update->set_where(output->clauses_size() - 1);
    }
    
    output->add_clauses()->set_allocated_update(update);
}

void TUpdateImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    const auto& update = input.clauses().at(startPoint).update();
    
    Updates_.clear();
    for (const auto& updateField : update.updates()) {
        TColumn column = CreateClauseFromProto(input, updateField.column_path());
        TClause expression = CreateClauseFromProto(input, updateField.expression());
        Updates_.emplace_back(column, expression);
    }
    
    if (update.has_where()) {
        Where_ = CreateClauseFromProto(input, update.where());
    }
}

TUpdate& TUpdate::AddUpdate(TColumn column, TClause expression) {
    std::dynamic_pointer_cast<TUpdateImpl>(Impl_)->Updates_.emplace_back(std::make_pair(column, expression));
    return *this;
}

TUpdate& TUpdate::Where(TClause conditions) {
    std::dynamic_pointer_cast<TUpdateImpl>(Impl_)->Where_ = conditions;
    return *this;
}

const std::vector<std::pair<TColumn, TClause>>& TUpdate::GetUpdates() const {
    return std::dynamic_pointer_cast<TUpdateImpl>(Impl_)->Updates_;
}

TClause TUpdate::GetWhere() const {
    return std::dynamic_pointer_cast<TUpdateImpl>(Impl_)->Where_;
}

////////////////////////////////////////////////////////////////////////////////
// TDelete implementation

void TDeleteImpl::ToProto(NApi::TQuery* output) const {
    auto deleteVal = new NApi::TDelete();
    
    if (Where_) {
        Where_.ToProto(output);
        deleteVal->set_where(output->clauses_size() - 1);
    }
    
    output->add_clauses()->set_allocated_delete_(deleteVal);
}

void TDeleteImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    const auto& deleteVal = input.clauses().at(startPoint).delete_();
    
    if (deleteVal.has_where()) {
        Where_ = CreateClauseFromProto(input, deleteVal.where());
    }
}

TDelete& TDelete::Where(TClause conditions) {
    std::dynamic_pointer_cast<TDeleteImpl>(Impl_)->Where_ = conditions;
    return *this;
}

TClause TDelete::GetWhere() const {
    return std::dynamic_pointer_cast<TDeleteImpl>(Impl_)->Where_;
}

////////////////////////////////////////////////////////////////////////////////
// TTruncate implementation

void TTruncateImpl::ToProto(NApi::TQuery* output) const {
    auto truncate = new NApi::TTruncate();
    truncate->set_table_num(TableNum_);
    
    output->add_clauses()->set_allocated_truncate(truncate);
}

void TTruncateImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    TableNum_ = input.clauses().at(startPoint).truncate().table_num();
}

TTruncate& TTruncate::SetTableNum(uint32_t tableNum) {
    std::dynamic_pointer_cast<TTruncateImpl>(Impl_)->TableNum_ = tableNum;
    return *this;
}

uint32_t TTruncate::GetTableNum() const {
    return std::dynamic_pointer_cast<TTruncateImpl>(Impl_)->TableNum_;
}

////////////////////////////////////////////////////////////////////////////////
// Transaction implementations

void TStartTransactionImpl::ToProto(NApi::TQuery* output) const {
    auto transaction = new NApi::TStartTransaction();
    output->add_clauses()->set_allocated_start_transaction(transaction);
}

void TStartTransactionImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    // Nothing to do for now
}

void TStartTransaction::ToProto(NOrm::NApi::TQuery* output) const {
    Impl_->ToProto(output);
}

void TStartTransaction::FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) {
    Impl_->FromProto(input, startPoint);
}

void TStartTransaction::SetTable(const std::string& table) {
    // Implementation placeholder
}

void TCommitTransactionImpl::ToProto(NApi::TQuery* output) const {
    auto transaction = new NApi::TCommitTransaction();
    output->add_clauses()->set_allocated_commit_transaction(transaction);
}

void TCommitTransactionImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    // Nothing to do
}

void TCommitTransaction::ToProto(NOrm::NApi::TQuery* output) const {
    Impl_->ToProto(output);
}

void TCommitTransaction::FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) {
    Impl_->FromProto(input, startPoint);
}

void TRollbackTransactionImpl::ToProto(NApi::TQuery* output) const {
    auto transaction = new NApi::TRollbackTransaction();
    output->add_clauses()->set_allocated_rollback_transaction(transaction);
}

void TRollbackTransactionImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    // Nothing to do
}

void TRollbackTransaction::ToProto(NOrm::NApi::TQuery* output) const {
    Impl_->ToProto(output);
}

void TRollbackTransaction::FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) {
    Impl_->FromProto(input, startPoint);
}

////////////////////////////////////////////////////////////////////////////////
// TQuery implementation

void TQueryImpl::ToProto(NApi::TQuery* output) const {
    for (auto& clause : Clauses_) {
        clause.ToProto(output);
        output->add_start_points(output->clauses_size() - 1);
    }
}

void TQueryImpl::FromProto(const NApi::TQuery& input) {
    Clauses_.clear();
    
    // Сначала создаем все клаузы
    for (const auto& startPoint : input.start_points()) {
        Clauses_.emplace_back(CreateClauseFromProto(input, startPoint));
    }
}

void TQuery::ToProto(NOrm::NApi::TQuery* output) const {
    Impl_->ToProto(output);
}

void TQuery::FromProto(const NOrm::NApi::TQuery& input) {
    Impl_->FromProto(input);
}

TQuery& TQuery::AddClause(TClause clause) {
    Impl_->Clauses_.push_back(clause);
    return *this;
}

const std::vector<TClause>& TQuery::GetClauses() const {
    return Impl_->Clauses_;
}

////////////////////////////////////////////////////////////////////////////////

TWhenCase::TWhenCase(TExpression expression)
    : Expression_(expression) {}

TThenCase TWhenCase::When(TClause condition) {
    Expression_.AddOperand(condition);
    return TThenCase(Expression_);
}

TClause TWhenCase::Else(TClause result) {
    Expression_.AddOperand(result);
    return Expression_;
}

TThenCase::TThenCase(TExpression expression)
    : Expression_(expression) {}

TWhenCase TThenCase::Then(TClause result) {
    Expression_.AddOperand(result);
    return TWhenCase(Expression_);
}

TWhenCase Case() {
    auto expression = TExpression();
    expression.SetExpressionType(NOrm::NQuery::EExpressionType::case_);
    return TWhenCase(expression);
}

////////////////////////////////////////////////////////////////////////////////
// Operators implementation

TClause operator+(TClause lhs, TClause rhs) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::add);
    expr.AddOperand(lhs);
    expr.AddOperand(rhs);
    return expr;
}

TClause operator-(TClause lhs, TClause rhs) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::subtract);
    expr.AddOperand(lhs);
    expr.AddOperand(rhs);
    return expr;
}

TClause operator*(TClause lhs, TClause rhs) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::multiply);
    expr.AddOperand(lhs);
    expr.AddOperand(rhs);
    return expr;
}

TClause operator/(TClause lhs, TClause rhs) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::divide);
    expr.AddOperand(lhs);
    expr.AddOperand(rhs);
    return expr;
}

TClause operator%(TClause lhs, TClause rhs) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::modulo);
    expr.AddOperand(lhs);
    expr.AddOperand(rhs);
    return expr;
}

TClause operator==(TClause lhs, TClause rhs) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::equals);
    expr.AddOperand(lhs);
    expr.AddOperand(rhs);
    return expr;
}

TClause operator!=(TClause lhs, TClause rhs) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::not_equals);
    expr.AddOperand(lhs);
    expr.AddOperand(rhs);
    return expr;
}

TClause operator<=(TClause lhs, TClause rhs) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::less_than_or_equals);
    expr.AddOperand(lhs);
    expr.AddOperand(rhs);
    return expr;
}

TClause operator>=(TClause lhs, TClause rhs) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::greater_than_or_equals);
    expr.AddOperand(lhs);
    expr.AddOperand(rhs);
    return expr;
}

TClause operator<(TClause lhs, TClause rhs) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::less_than);
    expr.AddOperand(lhs);
    expr.AddOperand(rhs);
    return expr;
}

TClause operator>(TClause lhs, TClause rhs) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::greater_than);
    expr.AddOperand(lhs);
    expr.AddOperand(rhs);
    return expr;
}

TClause operator&&(TClause lhs, TClause rhs) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::and_);
    expr.AddOperand(lhs);
    expr.AddOperand(rhs);
    return expr;
}

TClause operator||(TClause lhs, TClause rhs) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::or_);
    expr.AddOperand(lhs);
    expr.AddOperand(rhs);
    return expr;
}

TClause operator!(TClause element) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::not_);
    expr.AddOperand(element);
    return expr;
}

////////////////////////////////////////////////////////////////////////////////
// Function implementations

TClause Max(TClause element) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::max);
    expr.AddOperand(element);
    return expr;
}

TClause Min(TClause element) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::min);
    expr.AddOperand(element);
    return expr;
}

TClause Sum(TClause element) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::sum);
    expr.AddOperand(element);
    return expr;
}

TClause Avg(TClause element) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::avg);
    expr.AddOperand(element);
    return expr;
}

TClause Count(TClause element) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::count);
    expr.AddOperand(element);
    return expr;
}

// Mathematical functions
TClause Abs(TClause element) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::abs);
    expr.AddOperand(element);
    return expr;
}

TClause Round(TClause element) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::round);
    expr.AddOperand(element);
    return expr;
}

TClause Ceil(TClause element) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::ceil);
    expr.AddOperand(element);
    return expr;
}

TClause Floor(TClause element) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::floor);
    expr.AddOperand(element);
    return expr;
}

TClause Sqrt(TClause element) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::sqrt);
    expr.AddOperand(element);
    return expr;
}

TClause Log(TClause element) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::log);
    expr.AddOperand(element);
    return expr;
}

TClause Log(TClause element, TClause base) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::log);
    expr.AddOperand(element);
    expr.AddOperand(base);
    return expr;
}

TClause Rand() {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::random);
    return expr;
}

TClause Sin(TClause element) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::sin);
    expr.AddOperand(element);
    return expr;
}

TClause Cos(TClause element) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::cos);
    expr.AddOperand(element);
    return expr;
}

TClause Tan(TClause element) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::tan);
    expr.AddOperand(element);
    return expr;
}

TClause Pow(TClause base, TClause exp) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::power);
    expr.AddOperand(base);
    expr.AddOperand(exp);
    return expr;
}

// String functions
TClause Lower(TClause string) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::lower);
    expr.AddOperand(string);
    return expr;
}

std::string Lower(std::string element) { 
    std::transform(element.begin(), element.end(), element.begin(), [](unsigned char c){ return std::tolower(c); });
    return element;
}

TClause Upper(TClause string) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::upper);
    expr.AddOperand(string);
    return expr;
}

std::string Upper(std::string element) { 
    std::transform(element.begin(), element.end(), element.begin(), [](unsigned char c){ return std::toupper(c); });
    return element;
}

TClause SubStr(TClause string, TClause start, TClause n) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::substring);
    expr.AddOperand(string);
    expr.AddOperand(start);
    expr.AddOperand(n);
    return expr;
}

std::string SubStr(const std::string& string, size_t start, size_t n) {
    return string.substr(start, n);
}

TClause Len(TClause string) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::length);
    expr.AddOperand(string);
    return expr;
}

size_t Len(const std::string& string) {
    return string.length();
}

TClause Replace(TClause source, TClause match, TClause replace) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::replace);
    expr.AddOperand(source);
    expr.AddOperand(match);
    expr.AddOperand(replace);
    return expr;
}

std::string replace(std::string source, const std::string& match, const std::string& replace_str) {
    size_t pos = 0;
    while ((pos = source.find(match, pos)) != std::string::npos) {
        source.replace(pos, match.length(), replace_str);
        pos += replace_str.length();
    }
    return source;
}

TClause Trim(TClause string) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::trim);
    expr.AddOperand(string);
    return expr;
}

std::string Trim(std::string string) {
    string.erase(string.begin(), std::find_if(string.begin(), string.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    
    string.erase(std::find_if(string.rbegin(), string.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), string.end());
    
    return string;
}

TClause Left(TClause string, TClause n) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::left);
    expr.AddOperand(string);
    expr.AddOperand(n);
    return expr;
}

std::string Left(std::string string, size_t n) {
    return string.substr(0, n);
}

TClause Right(TClause string, TClause n) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::right);
    expr.AddOperand(string);
    expr.AddOperand(n);
    return expr;
}

std::string Right(std::string string, size_t n) {
    if (n >= string.length()) {
        return string;
    }
    return string.substr(string.length() - n);
}

TClause Pos(TClause substring, TClause string) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::position);
    expr.AddOperand(substring);
    expr.AddOperand(string);
    return expr;
}

size_t Pos(std::string substring, std::string string) {
    return string.find(substring);
}

TClause SplitPart(TClause string, TClause delim, TClause idx) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::split_part);
    expr.AddOperand(string);
    expr.AddOperand(delim);
    expr.AddOperand(idx);
    return expr;
}

std::string SplitPart(std::string string, std::string delim, size_t idx) {
    std::vector<std::string> parts;
    size_t start = 0;
    size_t end = 0;
    
    while ((end = string.find(delim, start)) != std::string::npos) {
        parts.push_back(string.substr(start, end - start));
        start = end + delim.length();
    }
    
    parts.push_back(string.substr(start));
    
    if (idx < parts.size()) {
        return parts[idx];
    }
    
    return "";
}

// Conditional functions
template <typename... Args>
TClause Coalesce(Args&&... args) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::coalesce);
    (expr.AddOperand(Val(std::forward<Args>(args))), ...);
    return expr;
}

template <typename... Args>
TClause Greatest(Args&&... args) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::greatest);
    (expr.AddOperand(Val(std::forward<Args>(args))), ...);
    return expr;
}

template <typename... Args>
TClause Least(Args&&... args) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::least);
    (expr.AddOperand(Val(std::forward<Args>(args))), ...);
    return expr;
}

// Factory functions
TClause Val(TClause clause) {
    return clause;
}

TString Val(const std::string& value) {
    auto string = TString();
    string.SetValue(value);
    return string;
}

TString Val(const char* value) {
    auto string = TString();
    string.SetValue(std::string(value));
    return string;
}

TInt Val(int32_t value) {
    auto integer = TInt();
    integer.SetValue(value);
    return integer;
}

TFloat Val(double value) {
    auto float_val = TFloat();
    float_val.SetValue(value);
    return float_val;
}

TBool Val(bool value) {
    auto bool_val = TBool();
    bool_val.SetValue(value);
    return bool_val;
}

TAll All() {
    return TAll();
}

TColumn Col(const TMessagePath& path) {
    auto column = TColumn();
    column.SetPath(path);
    column.SetType(NOrm::NQuery::EColumnType::ESingular);
    return column;
}

TColumn Excluded(const TMessagePath& path) {
    auto column = TColumn();
    column.SetPath(path);
    column.SetType(NOrm::NQuery::EColumnType::EExcluded);
    return column;
}

TDefault Default() {
    return TDefault();
}

TSelect Select() {
    return TSelect();
}

TInsert Insert() {
    return TInsert();
}

TUpdate Update() {
    return TUpdate();
}

TDelete Delete() {
    return TDelete();
}

TTruncate Truncate(uint32_t tableNum) {
    auto truncate = TTruncate();
    truncate.SetTableNum(tableNum);
    return truncate;
}

TQuery CreateQuery() {
    return TQuery();
}

} // namespace NOrm::NRelation
