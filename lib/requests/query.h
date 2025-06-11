#pragma once

#include <lib/requests/proto/query.pb.h>
#include <relation/base.h>
#include <relation/path.h>
#include <memory>
#include <string>
#include <regex>
#include <vector>

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

struct TClauseImpl {
    virtual ~TClauseImpl() = default;
    
    virtual void ToProto(NOrm::NApi::TQuery* output) const = 0;
    virtual void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) = 0;
};

class TClause {
public:
    TClause() : Impl_(nullptr) {}

    TClause(std::shared_ptr<TClauseImpl> impl)
        : Impl_(impl) {}

    template <typename T>
    requires std::is_base_of_v<TClause, T>
    TClause(const T& other)
        : Impl_(other.Impl_){}

    template <typename T>
    requires std::is_base_of_v<TClause, T>
    TClause(T&& other)
        : Impl_(other.Impl_){}

    template <typename T>
    requires std::is_base_of_v<TClause, T>
    TClause& operator=(const T& other) {
        Impl_ = other.Impl_;
        return *this;
    }

    template <typename T>
    requires std::is_base_of_v<TClause, T>
    TClause& operator=(T&& other) {
        if (Impl_ != other.Impl_) {
            Impl_ = other.Impl_;
        }
        return *this;
    }

    template <typename T>
    requires std::is_base_of_v<TClause, T>
    operator T() {
        T result;
        result.Impl_ = Impl_;
        return result;
    }

    operator bool() const {
        return bool(Impl_);
    }

    void ToProto(NOrm::NApi::TQuery* output) const {
        Impl_->ToProto(output);
    }
    void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) {
        Impl_->FromProto(input, startPoint);
    }

protected:
    std::shared_ptr<TClauseImpl> Impl_;
};

////////////////////////////////////////////////////////////////////////////////
// Базовые типы данных

struct TStringImpl : public TClauseImpl {
    void ToProto(NOrm::NApi::TQuery* output) const override;
    void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) override;
    
    std::string Value_;
};

class TString : public TClause {
public:
    TString() : TClause(std::make_shared<TStringImpl>()) {}
    
    TString& SetValue(const std::string& value);
    std::string GetValue() const;
};

struct TIntImpl : public TClauseImpl {
    void ToProto(NOrm::NApi::TQuery* output) const override;
    void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) override;
    
    int32_t Value_;
};

class TInt : public TClause {
public:
    TInt() : TClause(std::make_shared<TIntImpl>()) {}
    
    TInt& SetValue(int32_t value);
    int32_t GetValue() const;
};

struct TFloatImpl : public TClauseImpl {
    void ToProto(NOrm::NApi::TQuery* output) const override;
    void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) override;
    
    double Value_;
};

class TFloat : public TClause {
public:
    TFloat() : TClause(std::make_shared<TFloatImpl>()) {}
    
    TFloat& SetValue(double value);
    double GetValue() const;
};

struct TBoolImpl : public TClauseImpl {
    void ToProto(NOrm::NApi::TQuery* output) const override;
    void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) override;
    
    bool Value_;
};

class TBool : public TClause {
public:
    TBool() : TClause(std::make_shared<TBoolImpl>()) {}
    
    TBool& SetValue(bool value);
    bool GetValue() const;
};

TClause Val(TClause clause);
TString Val(const std::string& value);
TString Val(const char* value);
TInt Val(int32_t value);
TFloat Val(double value);
TBool Val(bool value);

////////////////////////////////////////////////////////////////////////////////
// Выражения и колонки

struct TExpressionImpl : public TClauseImpl {
    void ToProto(NOrm::NApi::TQuery* output) const override;
    void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) override;
    
    mutable std::vector<TClause> Operands_;
    NOrm::NQuery::EExpressionType ExpressionType_;
};

class TExpression : public TClause {
public:
    TExpression() : TClause(std::make_shared<TExpressionImpl>()) {}
    
    TExpression& SetExpressionType(NOrm::NQuery::EExpressionType type);
    TExpression& AddOperand(TClause operand);
    
    NOrm::NQuery::EExpressionType GetExpressionType() const;
    const std::vector<TClause>& GetOperands() const;
};

struct TAllImpl : public TClauseImpl {
    void ToProto(NOrm::NApi::TQuery* output) const override;
    void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) override;
};

class TAll : public TClause {
public:
    TAll() : TClause(std::make_shared<TAllImpl>()) {}
    
    void ToProto(NOrm::NApi::TQuery* output) const;
    void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint);
};

struct TColumnImpl : public TClauseImpl {
    void ToProto(NOrm::NApi::TQuery* output) const override;
    void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) override;
    
    TMessagePath Path_;
    NOrm::NQuery::EColumnType Type_;
};

class TColumn : public TClause {
public:
    TColumn() : TClause(std::make_shared<TColumnImpl>()) {}
    
    TColumn& SetPath(const TMessagePath& path);
    TColumn& SetType(NOrm::NQuery::EColumnType type);
    
    const TMessagePath& GetPath() const;
    NOrm::NQuery::EColumnType GetType() const;
};

struct TDefaultImpl : public TClauseImpl {
    void ToProto(NOrm::NApi::TQuery* output) const override;
    void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) override;
};

class TDefault : public TClause {
public:
    TDefault() : TClause(std::make_shared<TDefaultImpl>()) {}
    
    void ToProto(NOrm::NApi::TQuery* output) const;
    void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint);
};

////////////////////////////////////////////////////////////////////////////////
// Запросы SELECT

struct TSelectImpl : public TClauseImpl {
    void ToProto(NOrm::NApi::TQuery* output) const override;
    void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) override;
    
    std::vector<TClause> Selectors_;
    TClause Where_;
    TClause GroupBy_;
    TClause Having_;
    TClause OrderBy_;
    TClause Limit_;
};

class TSelect : public TClause {
public:
    TSelect() : TClause(std::make_shared<TSelectImpl>()) {}

    template <typename... Args>
    TSelect& Selectors(Args&&... args);
    
    TSelect& Where(TClause conditions);
    TSelect& GroupBy(TClause groupby);
    TSelect& Having(TClause having);
    TSelect& OrderBy(TClause orderBy);
    TSelect& Limit(TClause limit);
    template <typename T>
    TSelect& Limit(T limit) { return Limit(Val(limit)); }
};

////////////////////////////////////////////////////////////////////////////////
// Значения

struct TDefaultValuesImpl : public TClauseImpl {
    void ToProto(NOrm::NApi::TQuery* output) const override;
    void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) override;
};

class TDefaultValues : public TClause {
public:
    TDefaultValues() : TClause(std::make_shared<TDefaultValuesImpl>()) {}
    
    void ToProto(NOrm::NApi::TQuery* output) const;
    void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint);
};

struct TValueRowsImpl : public TClauseImpl {
    void ToProto(NOrm::NApi::TQuery* output) const override;
    void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) override;
    
    std::vector<std::vector<TClause>> Rows_;
};

class TValueRows : public TClause {
public:
    TValueRows() : TClause(std::make_shared<TValueRowsImpl>()) {}
    
    TValueRows& AddRow(const std::vector<TClause>& row);
    const std::vector<std::vector<TClause>>& GetRows() const;
};

////////////////////////////////////////////////////////////////////////////////
// Обработка конфликтов

struct TDoNothingImpl : public TClauseImpl {
    void ToProto(NOrm::NApi::TQuery* output) const override;
    void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) override;
};

class TDoNothing : public TClause {
public:
    TDoNothing() : TClause(std::make_shared<TDoNothingImpl>()) {}
    
    void ToProto(NOrm::NApi::TQuery* output) const;
    void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint);
};

struct TDoUpdateImpl : public TClauseImpl {
    void ToProto(NOrm::NApi::TQuery* output) const override;
    void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) override;
    
    std::vector<std::pair<TColumn, TClause>> Updates_;
};

class TDoUpdate : public TClause {
public:
    TDoUpdate() : TClause(std::make_shared<TDoUpdateImpl>()) {}
    
    TDoUpdate& AddUpdate(TColumn column, TClause expression);
    template <typename T>
    TDoUpdate& AddUpdate(TColumn column, T expression) { return AddUpdate(column, Val(expression)); }
    const std::vector<std::pair<TColumn, TClause>>& GetUpdates() const;
};

////////////////////////////////////////////////////////////////////////////////
// Операторы DML

struct TInsertImpl : public TClauseImpl {
    void ToProto(NOrm::NApi::TQuery* output) const override;
    void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) override;
    
    std::vector<TClause> Selectors_;
    TClause Values_;
    TClause OnConflict_;
};

class TInsert : public TClause {
public:
    TInsert() : TClause(std::make_shared<TInsertImpl>()) {}
    
    template <typename... Args>
    TInsert& Columns(Args&&... args);
    
    TInsert& Values(TClause values);
    TInsert& OnConflict(TClause action);

    TInsert& Default();

    template <typename... Args>
    TInsert& AddRow(Args&&... values) {
        auto impl = std::dynamic_pointer_cast<TInsertImpl>(Impl_);
        if (!impl->Values_) {
            impl->Values_ = TValueRows();
        }
        TValueRows(impl->Values_).AddRow(std::vector<TClause>({Val(std::forward<Args>(values))...}));
        return *this;
    }
    
    TInsert& DoNothing();
    TInsert& DoUpdate(TClause action);
};

struct TUpdateImpl : public TClauseImpl {
    void ToProto(NOrm::NApi::TQuery* output) const override;
    void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) override;
    
    std::vector<std::pair<TColumn, TClause>> Updates_;
    TClause Where_;
};

class TUpdate : public TClause {
public:
    TUpdate() : TClause(std::make_shared<TUpdateImpl>()) {}
    
    TUpdate& AddUpdate(TColumn column, TClause expression);
    template <typename T>
    TUpdate& AddUpdate(TColumn column, T expression) { return AddUpdate(column, Val(expression)); }
    TUpdate& Where(TClause conditions);
    
    const std::vector<std::pair<TColumn, TClause>>& GetUpdates() const;
    TClause GetWhere() const;
};

struct TDeleteImpl : public TClauseImpl {
    void ToProto(NOrm::NApi::TQuery* output) const override;
    void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) override;
    
    TClause Where_;
};

class TDelete : public TClause {
public:
    TDelete() : TClause(std::make_shared<TDeleteImpl>()) {}
    
    TDelete& Where(TClause conditions);
    TClause GetWhere() const;
};

struct TTruncateImpl : public TClauseImpl {
    void ToProto(NOrm::NApi::TQuery* output) const override;
    void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) override;
    
    uint32_t TableNum_;
};

class TTruncate : public TClause {
public:
    TTruncate() : TClause(std::make_shared<TTruncateImpl>()) {}
    
    TTruncate& SetTableNum(uint32_t tableNum);
    uint32_t GetTableNum() const;
};

struct TStartTransactionImpl : public TClauseImpl {
    void ToProto(NOrm::NApi::TQuery* output) const override;
    void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) override;
    
    uint32_t TableNum_;
};

class TStartTransaction : public TClause {
public:
    TStartTransaction() : TClause(std::make_shared<TStartTransactionImpl>()) {}
    
    void ToProto(NOrm::NApi::TQuery* output) const;
    void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint);
    void SetTable(const std::string& table);
};

struct TCommitTransactionImpl : public TClauseImpl {
    void ToProto(NOrm::NApi::TQuery* output) const override;
    void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) override;
};

class TCommitTransaction : public TClause {
public:
    TCommitTransaction() : TClause(std::make_shared<TCommitTransactionImpl>()) {}
    
    void ToProto(NOrm::NApi::TQuery* output) const;
    void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint);
};

struct TRollbackTransactionImpl : public TClauseImpl {
    void ToProto(NOrm::NApi::TQuery* output) const override;
    void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint) override;
};

class TRollbackTransaction : public TClause {
public:
    TRollbackTransaction() : TClause(std::make_shared<TRollbackTransactionImpl>()) {}
    
    void ToProto(NOrm::NApi::TQuery* output) const;
    void FromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint);
};

////////////////////////////////////////////////////////////////////////////////
// Корневые структуры

struct TQueryImpl {
    void ToProto(NOrm::NApi::TQuery* output) const;
    void FromProto(const NOrm::NApi::TQuery& input);
    
    std::vector<TClause> Clauses_;
};

class TQuery {
public:
    TQuery() {
        Impl_ = std::make_shared<TQueryImpl>();
    }
    
    void ToProto(NOrm::NApi::TQuery* output) const;
    void FromProto(const NOrm::NApi::TQuery& input);
    
    TQuery& AddClause(TClause clause);
    const std::vector<TClause>& GetClauses() const;

private:
    std::shared_ptr<TQueryImpl> Impl_;
};

using TQueryPtr = std::shared_ptr<TQuery>;

// Реализация шаблонных методов

template <typename... Args>
TSelect& TSelect::Selectors(Args&&... args) {
    auto impl = std::dynamic_pointer_cast<TSelectImpl>(Impl_);
    (impl->Selectors_.push_back(std::forward<Args>(args)), ...);
    return *this;
}

template <typename... Args>
TInsert& TInsert::Columns(Args&&... args) {
    auto impl = std::dynamic_pointer_cast<TInsertImpl>(Impl_);
    (impl->Selectors_.push_back(std::forward<Args>(args)), ...);
    return *this;
}

// Функции-фабрики для создания объектов
TAll All();
TColumn Col(const TMessagePath& path);
TColumn Excluded(const TMessagePath& path);
TDefault Default();
template <typename... Args>
TSelect Select(Args&&... args) {
    return TSelect().Selectors(std::forward<Args>(args)...);
}
TInsert Insert();
TUpdate Update();
TDelete Delete();
TTruncate Truncate(uint32_t tableNum);
TQuery CreateQuery();

////////////////////////////////////////////////////////////////////////////////

template <typename T> concept isClause = std::is_base_of_v<TClause, T>;
template <typename... Args> concept AllClause = (... && isClause<Args>);
template <typename... Args> concept AnyClause = (... || isClause<Args>);
template <typename... Args> concept Clausable = !AllClause<Args...> && AnyClause<Args...>;

// Арифметические операторы
TClause operator+(TClause lhs, TClause rhs);
template <typename T1, typename T2> requires Clausable<T1, T2> TClause operator+(T1 lhs, T2 rhs) { return Val(lhs) + Val(rhs); }

TClause operator-(TClause lhs, TClause rhs);
template <typename T1, typename T2> requires Clausable<T1, T2> TClause operator-(T1 lhs, T2 rhs) { return Val(lhs) - Val(rhs); }

TClause operator*(TClause lhs, TClause rhs);
template <typename T1, typename T2> requires Clausable<T1, T2> TClause operator*(T1 lhs, T2 rhs) { return Val(lhs) * Val(rhs); }

TClause operator/(TClause lhs, TClause rhs);
template <typename T1, typename T2> requires Clausable<T1, T2> TClause operator/(T1 lhs, T2 rhs) { return Val(lhs) / Val(rhs); }

TClause operator%(TClause lhs, TClause rhs);
template <typename T1, typename T2> requires Clausable<T1, T2> TClause operator%(T1 lhs, T2 rhs) { return Val(lhs) % Val(rhs); }

// Операторы сравнения
TClause operator==(TClause lhs, TClause rhs);
template <typename T1, typename T2> requires Clausable<T1, T2> TClause operator==(T1 lhs, T2 rhs) { return Val(lhs) == Val(rhs); }

TClause operator!=(TClause lhs, TClause rhs);
template <typename T1, typename T2> requires Clausable<T1, T2> TClause operator!=(T1 lhs, T2 rhs) { return Val(lhs) != Val(rhs); }

TClause operator<=(TClause lhs, TClause rhs);
template <typename T1, typename T2> requires Clausable<T1, T2> TClause operator<=(T1 lhs, T2 rhs) { return Val(lhs) <= Val(rhs); }

TClause operator>=(TClause lhs, TClause rhs);
template <typename T1, typename T2> requires Clausable<T1, T2> TClause operator>=(T1 lhs, T2 rhs) { return Val(lhs) >= Val(rhs); }

TClause operator<(TClause lhs, TClause rhs);
template <typename T1, typename T2> requires Clausable<T1, T2> TClause operator<(T1 lhs, T2 rhs) { return Val(lhs) < Val(rhs); }

TClause operator>(TClause lhs, TClause rhs);
template <typename T1, typename T2> requires Clausable<T1, T2> TClause operator>(T1 lhs, T2 rhs) { return Val(lhs) > Val(rhs); }

// Логические операторы
TClause operator&&(TClause lhs, TClause rhs);
template <typename T1, typename T2> requires Clausable<T1, T2> TClause operator&&(T1 lhs, T2 rhs) { return Val(lhs) && Val(rhs); }

TClause operator||(TClause lhs, TClause rhs);
template <typename T1, typename T2> requires Clausable<T1, T2> TClause operator||(T1 lhs, T2 rhs) { return Val(lhs) || Val(rhs); }

TClause operator!(TClause element);

// Агрегатные функции
TClause Max(TClause element);
TClause Min(TClause element);
TClause Sum(TClause element);
TClause Avg(TClause element);
TClause Count(TClause element);

// Математические функции
TClause Abs(TClause element);
template <typename T> constexpr auto Abs(const T& element) { return std::abs(element); }

TClause Round(TClause element);
template <typename T> constexpr auto Round(const T& element) { return std::round(element); }
TClause Ceil(TClause element);
template <typename T> constexpr auto Ceil(const T& element) { return std::ceil(element); }
TClause Floor(TClause element);
template <typename T> constexpr auto Floor(const T& element) { return std::floor(element); }
TClause Sqrt(TClause element);
template <typename T> constexpr auto Sqrt(const T& element) { return std::sqrt(element); }
TClause Log(TClause element);
template <typename T> constexpr auto Log(const T& element) { return std::log(element); }
TClause Log(TClause element, TClause base);
template <typename T1, typename T2> requires Clausable<T1, T2> TClause Log(T1 element, T2 base) { return Log(Val(element), Val(base)); }
template <typename T1, typename T2> requires (!AnyClause<T1, T2>) constexpr auto Log(T1 element, T2 base) { return std::log(element, base); }

TClause Rand();
TClause Sin(TClause element);
template <typename T> constexpr auto Sin(const T& element) { return std::sin(element); }
TClause Cos(TClause element);
template <typename T> constexpr auto Cos(const T& element) { return std::cos(element); }
TClause Tan(TClause element);
template <typename T> constexpr auto Tan(const T& element) { return std::tan(element); }
TClause Pow(TClause base, TClause exp);
template <typename T1, typename T2> requires Clausable<T1, T2> TClause Pow(T1 base, T2 exp) { return Pow(Val(base), Val(exp)); }
template <typename T1, typename T2> requires (!AnyClause<T1, T2>) constexpr auto Pow(T1 element, T2 base) { return std::pow(element, base); }

// Строковые функции
TClause Lower(TClause string);
std::string Lower(std::string element);
TClause Upper(TClause string);
std::string Upper(std::string element);
TClause SubStr(TClause string, TClause start, TClause n);
template <typename T1, typename T2, typename T3> requires (Clausable<T1, T2, T3>)
TClause SubStr(T1 string, T2 start, T3 n) { return SubStr(Val(string), Val(start), Val(n)); }
std::string SubStr(const std::string& string, size_t start, size_t n);

TClause Len(TClause string);
size_t Len(const std::string& string);
TClause Replace(TClause source, TClause match, TClause replace);
template <typename T1, typename T2, typename T3> requires (Clausable<T1, T2, T3>)
TClause Replace(T1 source, T2 match, T3 replace) { return ::NOrm::NRelation::Replace(Val(source), Val(match), Val(replace)); }

TClause Trim(TClause string);
std::string Trim(std::string string);
TClause Left(TClause string, TClause n);
template <typename T1, typename T2> requires Clausable<T1, T2> TClause Left(T1 string, T2 n) { return Left(Val(string), Val(n)); }
std::string Left(std::string string, size_t n);
TClause Right(TClause string, TClause n);
template <typename T1, typename T2> requires Clausable<T1, T2> TClause Right(T1 string, T2 n) { return Right(Val(string), Val(n)); }
std::string Right(std::string string, size_t n);
TClause Pos(TClause substring, TClause string);
template <typename T1, typename T2> requires Clausable<T1, T2> TClause Pos(T1 substring, T2 string) { return Pos(Val(substring), Val(string)); }
size_t Pos(std::string substring, std::string string);
TClause SplitPart(TClause string, TClause delim, TClause idx);
template <typename T1, typename T2, typename T3> requires (Clausable<T1, T2, T3>)
TClause SplitPart(T1 string, T2 delim, T3 idx) { return SplitPart(Val(string), Val(delim), Val(idx)); }
std::string SplitPart(std::string string, std::string delim, size_t idx);

class TThenCase;

class TWhenCase {
public:
    TWhenCase(TExpression expression);
    [[nodiscard]] TThenCase When(TClause condition);
    TClause Else(TClause result);

    operator TClause() {
        return Expression_;
    }

private:
    TExpression Expression_;
};

class TThenCase {
public:
    TThenCase(TExpression expression);
    [[nodiscard]] TWhenCase Then(TClause result);

private:
    TExpression Expression_;
};

[[nodiscard]] TWhenCase Case();

template <typename... Args>
TClause Coalesce(Args&&... args);

template <typename... Args>
TClause Greatest(Args&&... args);

template <typename... Args>
TClause Least(Args&&... args);

////////////////////////////////////////////////////////////////////////////////

TClause CreateClauseFromProto(const NOrm::NApi::TQuery& input, uint32_t startPoint);

} // namespace NOrm::NRelation
