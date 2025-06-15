#include <requests/query.h>
#include <relation/relation_manager.h>
#include <google/protobuf/dynamic_message.h>

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

NOrm::NApi::TClause::ValueCase TClauseImpl::Type() const {
    return NOrm::NApi::TClause::VALUE_NOT_SET;
}

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

void TStringImpl::ToProto(NApi::TQuery* output) const {
    auto string = new NApi::TString();
    string->set_value(Value_);
    output->add_clauses()->set_allocated_string(string);
}

void TStringImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    Value_ = input.clauses().at(startPoint).string().value();
}

NOrm::NApi::TClause::ValueCase TStringImpl::Type() const {
    return NOrm::NApi::TClause::ValueCase::kString;
}

TString& TString::SetValue(const std::string& value) {
    std::dynamic_pointer_cast<TStringImpl>(Impl_)->Value_ = value;
    return *this;
}

std::string TString::GetValue() const {
    return std::dynamic_pointer_cast<TStringImpl>(Impl_)->Value_;
}

////////////////////////////////////////////////////////////////////////////////

void TIntImpl::ToProto(NApi::TQuery* output) const {
    auto integer = new NApi::TInt();
    integer->set_value(Value_);
    output->add_clauses()->set_allocated_integer(integer);
}

void TIntImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    Value_ = input.clauses().at(startPoint).integer().value();
}

NOrm::NApi::TClause::ValueCase TIntImpl::Type() const {
    return NOrm::NApi::TClause::ValueCase::kInteger;
}

TInt& TInt::SetValue(int32_t value) {
    std::dynamic_pointer_cast<TIntImpl>(Impl_)->Value_ = value;
    return *this;
}

int32_t TInt::GetValue() const {
    return std::dynamic_pointer_cast<TIntImpl>(Impl_)->Value_;
}

////////////////////////////////////////////////////////////////////////////////

void TFloatImpl::ToProto(NApi::TQuery* output) const {
    auto float_val = new NApi::TFloat();
    float_val->set_value(Value_);
    output->add_clauses()->set_allocated_float_(float_val);
}

void TFloatImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    Value_ = input.clauses().at(startPoint).float_().value();
}

NOrm::NApi::TClause::ValueCase TFloatImpl::Type() const {
    return NOrm::NApi::TClause::ValueCase::kFloat;
}

TFloat& TFloat::SetValue(double value) {
    std::dynamic_pointer_cast<TFloatImpl>(Impl_)->Value_ = value;
    return *this;
}

double TFloat::GetValue() const {
    return std::dynamic_pointer_cast<TFloatImpl>(Impl_)->Value_;
}

////////////////////////////////////////////////////////////////////////////////

void TBoolImpl::ToProto(NApi::TQuery* output) const {
    auto bool_val = new NApi::TBool();
    bool_val->set_value(Value_);
    output->add_clauses()->set_allocated_bool_(bool_val);
}

void TBoolImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    Value_ = input.clauses().at(startPoint).bool_().value();
}

NOrm::NApi::TClause::ValueCase TBoolImpl::Type() const {
    return NOrm::NApi::TClause::ValueCase::kBool;
}

TBool& TBool::SetValue(bool value) {
    std::dynamic_pointer_cast<TBoolImpl>(Impl_)->Value_ = value;
    return *this;
}

bool TBool::GetValue() const {
    return std::dynamic_pointer_cast<TBoolImpl>(Impl_)->Value_;
}

////////////////////////////////////////////////////////////////////////////////

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

NOrm::NApi::TClause::ValueCase TExpressionImpl::Type() const {
    return NOrm::NApi::TClause::ValueCase::kExpression;
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

void TAllImpl::ToProto(NApi::TQuery* output) const {
    output->add_clauses()->mutable_all();
}

void TAllImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
}

NOrm::NApi::TClause::ValueCase TAllImpl::Type() const {
    return NOrm::NApi::TClause::ValueCase::kAll;
}

void TAll::ToProto(NOrm::NApi::TQuery* output) const {
    Impl_->ToProto(output);
}

void TAll::FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) {
    Impl_->FromProto(input, startPoint);
}

////////////////////////////////////////////////////////////////////////////////

void TColumnImpl::ToProto(NApi::TQuery* output) const {
    auto columnVal = new NApi::TColumn();
    for (const auto& num : Path_) {
        columnVal->add_field_path(num);
    }
    columnVal->set_type(Type_);
    
    output->add_clauses()->set_allocated_column(columnVal);
}

void TColumnImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    const auto& column = input.clauses().at(startPoint).column();
    Path_ = TMessagePath(column.field_path().begin(), column.field_path().end());
    Type_ = column.type();
}

NOrm::NApi::TClause::ValueCase TColumnImpl::Type() const {
    return NOrm::NApi::TClause::ValueCase::kColumn;
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

void TDefaultImpl::ToProto(NApi::TQuery* output) const {
    output->add_clauses()->mutable_default_();
}

void TDefaultImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
}

NOrm::NApi::TClause::ValueCase TDefaultImpl::Type() const {
    return NOrm::NApi::TClause::ValueCase::kDefault;
}

void TDefault::ToProto(NOrm::NApi::TQuery* output) const {
    Impl_->ToProto(output);
}

void TDefault::FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) {
    Impl_->FromProto(input, startPoint);
}

////////////////////////////////////////////////////////////////////////////////

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

NOrm::NApi::TClause::ValueCase TSelectImpl::Type() const {
    return NOrm::NApi::TClause::ValueCase::kSelect;
}

TSelect& TSelect::SetTableNum(uint32_t tableNum) {
    std::dynamic_pointer_cast<TSelectImpl>(Impl_)->Table_ = tableNum;
    return *this;
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

uint32_t TSelect::GetTableNum() const {
    return std::dynamic_pointer_cast<TSelectImpl>(Impl_)->Table_;
}

const std::vector<TClause>& TSelect::GetSelectors() const {
    return std::dynamic_pointer_cast<TSelectImpl>(Impl_)->Selectors_;
}

TClause TSelect::GetWhere() const {
    return std::dynamic_pointer_cast<TSelectImpl>(Impl_)->Where_;
}

TClause TSelect::GetGroupBy() const {
    return std::dynamic_pointer_cast<TSelectImpl>(Impl_)->GroupBy_;
}

TClause TSelect::GetHaving() const {
    return std::dynamic_pointer_cast<TSelectImpl>(Impl_)->Having_;
}

TClause TSelect::GetOrderBy() const {
    return std::dynamic_pointer_cast<TSelectImpl>(Impl_)->OrderBy_;
}

TClause TSelect::GetLimit() const {
    return std::dynamic_pointer_cast<TSelectImpl>(Impl_)->Limit_;
}

////////////////////////////////////////////////////////////////////////////////

TAttribute::TAttribute() 
    : Path()
    , Data() {
}

TAttribute::TAttribute(const TMessagePath& path, bool value)
    : Path(path)
    , Data(value) {
}

TAttribute::TAttribute(const TMessagePath& path, uint32_t value)
    : Path(path)
    , Data(value) {
}

TAttribute::TAttribute(const TMessagePath& path, int32_t value)
    : Path(path)
    , Data(value) {
}

TAttribute::TAttribute(const TMessagePath& path, uint64_t value)
    : Path(path)
    , Data(value) {
}

TAttribute::TAttribute(const TMessagePath& path, int64_t value)
    : Path(path)
    , Data(value) {
}

TAttribute::TAttribute(const TMessagePath& path, float value)
    : Path(path)
    , Data(value) {
}

TAttribute::TAttribute(const TMessagePath& path, double value)
    : Path(path)
    , Data(value) {
}

TAttribute::TAttribute(const TMessagePath& path, const std::string& value)
    : Path(path)
    , Data(value) {
}

void TAttribute::FromProto(const NOrm::NApi::TAttribute& attr) {
    static google::protobuf::DynamicMessageFactory factory;
    auto& relationManager = NRelation::TRelationManager::GetInstance();

    Path = TMessagePath(attr.path().begin(), attr.path().end());
    auto type = relationManager.GetObjectType(Path);
    if (type & EObjectType::Message) {
        const google::protobuf::Message* prototype = factory.GetPrototype(relationManager.GetMessage(Path)->GetMessageDescriptor());

        SetMessage(prototype->New());
        ASSERT(GetMessage()->ParseFromString(attr.payload()), "Failed to parse attribute in {}", Path);
    } else {
        const auto& typeInfo = relationManager.GetPrimitiveField(Path)->GetTypeInfo();
        if (std::holds_alternative<TBoolFieldInfo>(typeInfo)) {
            SetBool(attr.payload()[0] != 0);
        } else if (std::holds_alternative<TInt32FieldInfo>(typeInfo)) {
            int32_t value;
            std::memcpy(&value, attr.payload().data(), sizeof(int32_t));
            SetInt32(value);
        } else if (std::holds_alternative<TUInt32FieldInfo>(typeInfo)) {
            uint32_t value;
            std::memcpy(&value, attr.payload().data(), sizeof(uint32_t));
            SetUint32(value);
        } else if (std::holds_alternative<TUInt64FieldInfo>(typeInfo)) {
            uint64_t value;
            std::memcpy(&value, attr.payload().data(), sizeof(uint64_t));
            SetUint64(value);
        } else if (std::holds_alternative<TInt64FieldInfo>(typeInfo)) {
            int64_t value;
            std::memcpy(&value, attr.payload().data(), sizeof(int64_t));
            SetInt64(value);
        } else if (std::holds_alternative<TFloatFieldInfo>(typeInfo)) {
            float value;
            std::memcpy(&value, attr.payload().data(), sizeof(float));
            SetFloat(value);
        } else if (std::holds_alternative<TDoubleFieldInfo>(typeInfo)) {
            double value;
            std::memcpy(&value, attr.payload().data(), sizeof(double));
            SetDouble(value);
        } else if (std::holds_alternative<TStringFieldInfo>(typeInfo)) {
            std::string value(attr.payload().begin(), attr.payload().end());
            SetString(value);
        }
    }
}

NOrm::NApi::TAttribute TAttribute::ToProto() const {
    NOrm::NApi::TAttribute attribute;
    for (auto e : Path.data()) {
        attribute.add_path(e);
    }

    if (std::holds_alternative<bool>(Data)) {
        uint8_t value = std::get<bool>(Data) ? 1 : 0;
        attribute.set_payload(std::string(reinterpret_cast<char*>(&value), sizeof(uint8_t)));
    } else if (std::holds_alternative<uint32_t>(Data)) {
        uint32_t value = std::get<uint32_t>(Data);
        attribute.set_payload(std::string(reinterpret_cast<char*>(&value), sizeof(uint32_t)));
    } else if (std::holds_alternative<int32_t>(Data)) {
        int32_t value = std::get<int32_t>(Data);
        attribute.set_payload(std::string(reinterpret_cast<char*>(&value), sizeof(int32_t)));
    } else if (std::holds_alternative<uint64_t>(Data)) {
        uint64_t value = std::get<uint64_t>(Data);
        attribute.set_payload(std::string(reinterpret_cast<char*>(&value), sizeof(uint64_t)));
    } else if (std::holds_alternative<int64_t>(Data)) {
        int64_t value = std::get<int64_t>(Data);
        attribute.set_payload(std::string(reinterpret_cast<char*>(&value), sizeof(int64_t)));
    } else if (std::holds_alternative<float>(Data)) {
        float value = std::get<float>(Data);
        attribute.set_payload(std::string(reinterpret_cast<char*>(&value), sizeof(float)));
    } else if (std::holds_alternative<double>(Data)) {
        double value = std::get<double>(Data);
        attribute.set_payload(std::string(reinterpret_cast<char*>(&value), sizeof(double)));
    } else if (std::holds_alternative<std::string>(Data)) {
        attribute.set_payload(std::get<std::string>(Data));
    } else if (std::holds_alternative<std::shared_ptr<google::protobuf::Message>>(Data)) {
        std::string serialized;
        std::get<std::shared_ptr<google::protobuf::Message>>(Data)->SerializeToString(&serialized);
        attribute.set_payload(serialized);
    }

    return attribute;
}

////////////////////////////////////////////////////////////////////////////////

void TInsertImpl::ToProto(NApi::TQuery* output) const {
    auto insert = new NApi::TInsert();
    insert->set_table_num(TableNum_);
    insert->set_update_if_exists(UpdateIfExists_);
    
    for (const auto& subrequest : Subrequests_) {
        auto subRequestProto = insert->add_subrequests();
        for (const auto& attr : subrequest) {
            *subRequestProto->add_attributes() = attr.ToProto();
        }
    }
    
    output->add_clauses()->set_allocated_insert(insert);
}

void TInsertImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    const auto& insert = input.clauses().at(startPoint).insert();
    
    TableNum_ = insert.table_num();
    UpdateIfExists_ = insert.update_if_exists();
    
    Subrequests_.clear();
    for (const auto& subrequest : insert.subrequests()) {
        std::vector<TAttribute> attributes;
        for (const auto& attr : subrequest.attributes()) {
            attributes.emplace_back().FromProto(attr);
        }
        Subrequests_.push_back(attributes);
    }
}

NOrm::NApi::TClause::ValueCase TInsertImpl::Type() const {
    return NOrm::NApi::TClause::ValueCase::kInsert;
}

TInsert& TInsert::SetTableNum(uint32_t tableNum) {
    std::dynamic_pointer_cast<TInsertImpl>(Impl_)->TableNum_ = tableNum;
    return *this;
}

TInsert& TInsert::AddSubrequest(const std::vector<TAttribute>& attributes) {
    std::dynamic_pointer_cast<TInsertImpl>(Impl_)->Subrequests_.push_back(attributes);
    return *this;
}

TInsert& TInsert::UpdateIfExists() {
    std::dynamic_pointer_cast<TInsertImpl>(Impl_)->UpdateIfExists_ = true;
    return *this;
}

uint32_t TInsert::GetTableNum() const {
    return std::dynamic_pointer_cast<TInsertImpl>(Impl_)->TableNum_;
}

const std::vector<std::vector<TAttribute>>& TInsert::GetSubrequests() const {
    return std::dynamic_pointer_cast<TInsertImpl>(Impl_)->Subrequests_;
}

bool TInsert::GetUpdateIfExists() const {
    return std::dynamic_pointer_cast<TInsertImpl>(Impl_)->UpdateIfExists_;
}

////////////////////////////////////////////////////////////////////////////////

void TUpdateImpl::ToProto(NApi::TQuery* output) const {
    auto update = new NApi::TUpdate();
    update->set_table_num(TableNum_);
    
    for (const auto& updateSet : Updates_) {
        auto updateSubrequest = update->add_updates();
        for (const auto& attr : updateSet) {
            *updateSubrequest->add_attributes() = attr.ToProto();
        }
    }
    
    output->add_clauses()->set_allocated_update(update);
}

void TUpdateImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    const auto& update = input.clauses().at(startPoint).update();
    
    TableNum_ = update.table_num();
    
    Updates_.clear();
    for (const auto& updateSubrequest : update.updates()) {
        std::vector<TAttribute> attributes;
        for (const auto& attr : updateSubrequest.attributes()) {
            attributes.emplace_back().FromProto(attr);
        }
        Updates_.push_back(attributes);
    }
}

TUpdate& TUpdate::SetTableNum(uint32_t tableNum) {
    std::dynamic_pointer_cast<TUpdateImpl>(Impl_)->TableNum_ = tableNum;
    return *this;
}

NOrm::NApi::TClause::ValueCase TUpdateImpl::Type() const {
    return NOrm::NApi::TClause::ValueCase::kUpdate;
}

TUpdate& TUpdate::AddUpdate(const std::vector<TAttribute>& attributes) {
    std::dynamic_pointer_cast<TUpdateImpl>(Impl_)->Updates_.push_back(attributes);
    return *this;
}

uint32_t TUpdate::GetTableNum() const {
    return std::dynamic_pointer_cast<TUpdateImpl>(Impl_)->TableNum_;
}

const std::vector<std::vector<TAttribute>>& TUpdate::GetUpdates() const {
    return std::dynamic_pointer_cast<TUpdateImpl>(Impl_)->Updates_;
}

////////////////////////////////////////////////////////////////////////////////

void TDeleteImpl::ToProto(NApi::TQuery* output) const {
    auto deleteVal = new NApi::TDelete();
    deleteVal->set_table_num(TableNum_);
    
    if (Where_) {
        Where_.ToProto(output);
        deleteVal->set_where(output->clauses_size() - 1);
    }
    
    output->add_clauses()->set_allocated_delete_(deleteVal);
}

void TDeleteImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    const auto& deleteVal = input.clauses().at(startPoint).delete_();
    
    TableNum_ = deleteVal.table_num();
    
    if (deleteVal.has_where()) {
        Where_ = CreateClauseFromProto(input, deleteVal.where());
    }
}

NOrm::NApi::TClause::ValueCase TDeleteImpl::Type() const {
    return NOrm::NApi::TClause::ValueCase::kDelete;
}

TDelete& TDelete::Where(TClause conditions) {
    std::dynamic_pointer_cast<TDeleteImpl>(Impl_)->Where_ = conditions;
    return *this;
}

TDelete& TDelete::SetTableNum(uint32_t tableNum) {
    std::dynamic_pointer_cast<TDeleteImpl>(Impl_)->TableNum_ = tableNum;
    return *this;
}

uint32_t TDelete::GetTableNum() const {
    return std::dynamic_pointer_cast<TDeleteImpl>(Impl_)->TableNum_;
}

TClause TDelete::GetWhere() const {
    return std::dynamic_pointer_cast<TDeleteImpl>(Impl_)->Where_;
}

////////////////////////////////////////////////////////////////////////////////

void TTruncateImpl::ToProto(NApi::TQuery* output) const {
    auto truncate = new NApi::TTruncate();
    truncate->set_table_num(TableNum_);
    
    output->add_clauses()->set_allocated_truncate(truncate);
}

void TTruncateImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
    TableNum_ = input.clauses().at(startPoint).truncate().table_num();
}

NOrm::NApi::TClause::ValueCase TTruncateImpl::Type() const {
    return NOrm::NApi::TClause::ValueCase::kTruncate;
}

TTruncate& TTruncate::SetTableNum(uint32_t tableNum) {
    std::dynamic_pointer_cast<TTruncateImpl>(Impl_)->TableNum_ = tableNum;
    return *this;
}

uint32_t TTruncate::GetTableNum() const {
    return std::dynamic_pointer_cast<TTruncateImpl>(Impl_)->TableNum_;
}

////////////////////////////////////////////////////////////////////////////////

void TStartTransactionImpl::ToProto(NApi::TQuery* output) const {
    auto transaction = new NApi::TStartTransaction();
    output->add_clauses()->set_allocated_start_transaction(transaction);
}

void TStartTransactionImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
}

NOrm::NApi::TClause::ValueCase TStartTransactionImpl::Type() const {
    return NOrm::NApi::TClause::ValueCase::kStartTransaction;
}

void TStartTransaction::ToProto(NOrm::NApi::TQuery* output) const {
    Impl_->ToProto(output);
}

void TStartTransaction::FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) {
    Impl_->FromProto(input, startPoint);
}

void TStartTransaction::SetTable(const std::string& table) {
}

void TCommitTransactionImpl::ToProto(NApi::TQuery* output) const {
    auto transaction = new NApi::TCommitTransaction();
    output->add_clauses()->set_allocated_commit_transaction(transaction);
}

void TCommitTransactionImpl::FromProto(const NApi::TQuery& input, uint32_t startPoint) {
}

NOrm::NApi::TClause::ValueCase TCommitTransactionImpl::Type() const {
    return NOrm::NApi::TClause::ValueCase::kCommitTransaction;
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
}

NOrm::NApi::TClause::ValueCase TRollbackTransactionImpl::Type() const {
    return NOrm::NApi::TClause::ValueCase::kRollbackTransaction;
}

void TRollbackTransaction::ToProto(NOrm::NApi::TQuery* output) const {
    Impl_->ToProto(output);
}

void TRollbackTransaction::FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) {
    Impl_->FromProto(input, startPoint);
}

////////////////////////////////////////////////////////////////////////////////

void TQueryImpl::ToProto(NApi::TQuery* output) const {
    for (auto& clause : Clauses_) {
        clause.ToProto(output);
        output->add_start_points(output->clauses_size() - 1);
    }
}

void TQueryImpl::FromProto(const NApi::TQuery& input) {
    Clauses_.clear();
    
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

TClause In(TClause element, TClause group) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::in);
    expr.AddOperand(element);
    expr.AddOperand(group);
    return expr;
}

TClause Exists(TClause subquery) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::exists);
    expr.AddOperand(subquery);
    return expr;
}

////////////////////////////////////////////////////////////////////////////////

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

TClause Like(TClause string, TClause pattern) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::like);
    expr.AddOperand(string);
    expr.AddOperand(pattern);
    return expr;
}

TClause Ilike(TClause string, TClause pattern) {
    auto expr = TExpression();
    expr.SetExpressionType(NOrm::NQuery::EExpressionType::ilike);
    expr.AddOperand(string);
    expr.AddOperand(pattern);
    return expr;
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

TInsert Insert(const std::string& path) {
    return Insert(TMessagePath(path));
}

TInsert Insert(const TMessagePath& path) {
    auto clause = TInsert();
    clause.SetTableNum(path.back());
    return clause;
}

TUpdate Update(const std::string& path) {
    return Update(TMessagePath(path));
}

TUpdate Update(const TMessagePath& path) {
    auto clause = TUpdate();
    clause.SetTableNum(path.back());
    return clause;
}

TDelete Delete(const std::string& path) {
    return Delete(TMessagePath(path));
}

TDelete Delete(const TMessagePath& path) {
    auto clause = TDelete();
    clause.SetTableNum(path.back());
    return clause;
}

TTruncate Truncate(const std::string& path) {
    return Truncate(TMessagePath(path));
}

TTruncate Truncate(const TMessagePath& path) {
    auto clause = TTruncate();
    clause.SetTableNum(path.back());
    return clause;
}


TQuery CreateQuery() {
    return TQuery();
}

} // namespace NOrm::NRelation
