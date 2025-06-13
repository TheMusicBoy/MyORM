#pragma once

#include <query_builder/builder_base.h>
#include <relation/relation_manager.h>
#include <memory>
#include <string>
#include <vector>

namespace NOrm::NRelation::Builder {

namespace {

////////////////////////////////////////////////////////////////////////////////

template <typename T>
class StackWrapper {
private:
    std::vector<T> stack;

public:
    // StackGuard для автоматического удаления элемента
    class StackGuard {
    private:
        std::vector<T>& stack;
        bool active;

    public:
        explicit StackGuard(std::vector<T>& s) : stack(s), active(true) {}
        
        // Конструктор перемещения
        StackGuard(StackGuard&& other) noexcept : stack(other.stack), active(other.active) {
            other.active = false;
        }
        
        // Запрещаем копирование
        StackGuard(const StackGuard&) = delete;
        StackGuard& operator=(const StackGuard&) = delete;
        StackGuard& operator=(StackGuard&&) = delete;
        
        ~StackGuard() {
            if (active && !stack.empty()) {
                stack.pop_back();
            }
        }
    };

    // Стандартные методы доступа к стеку
    bool empty() const { return stack.empty(); }
    size_t size() const { return stack.size(); }
    T& top() { return stack.back(); }
    const T& top() const { return stack.back(); }
    T& bottom() { return stack.front(); }
    const T& bottom() const { return stack.front(); }
    void pop() { stack.pop_back(); }

    T& at(size_t idx) {
        ASSERT(idx < stack.size(), "Out of range");
        return stack.at(stack.size() - idx - 1);
    }

    const T& at(size_t idx) const {
        ASSERT(idx < stack.size(), "Out of range");
        return stack.at(stack.size() - idx - 1);
    }

    // Методы, возвращающие StackGuard
    template <typename U>
    StackGuard push(U&& value) {
        stack.push_back(std::forward<U>(value));
        return StackGuard(stack);
    }

    template <typename... Args>
    StackGuard emplace(Args&&... args) {
        stack.emplace_back(std::forward<Args>(args)...);
        return StackGuard(stack);
    }
};

////////////////////////////////////////////////////////////////////////////////

} // namespace

class TPostgresBuilder : public TBuilderBase, public std::enable_shared_from_this<TPostgresBuilder> {
public:
    TPostgresBuilder();
    ~TPostgresBuilder() override;

    // Базовые типы данных
    std::string BuildString(const std::string& value) override;
    std::string BuildInt(int32_t value) override;
    std::string BuildFloat(double value) override;
    std::string BuildBool(bool value) override;

    // Выражения и колонки
    std::string BuildExpression(
        NQuery::EExpressionType type,
        const std::vector<TClausePtr>& operands) override;
    std::string BuildAll() override;
    std::string BuildColumn(
        const TMessagePath& path,
        NQuery::EColumnType type) override;
    std::string BuildTable(const TMessagePath& path) override;
    std::string BuildDefault() override;
    
    std::string BuildJoin(TMessagePath table, TClausePtr condition, TJoin::EJoinType type) override;

    // Запросы SELECT
    std::string BuildSelect(
        const std::vector<TClausePtr>& selectors,
        const std::vector<TClausePtr>& from,
        const std::vector<TClausePtr>& join,
        TClausePtr where,
        TClausePtr groupBy,
        TClausePtr having,
        TClausePtr orderBy,
        TClausePtr limit) override;
    
    // Запросы INSERT
    std::string BuildDefaultValueList() override;
    std::string BuildRowValues(const std::vector<std::vector<TClausePtr>>& rows) override;
    std::string BuildDoNothing() override;
    std::string BuildDoUpdate(const std::vector<std::pair<TClausePtr, TClausePtr>>& updates) override;
    std::string BuildInsert(
        const TMessagePath& table,
        const std::vector<TClausePtr>& selectors,
        bool isValues,
        const std::vector<std::vector<TClausePtr>>& values,
        bool isDoUpdate,
        const std::vector<std::pair<TClausePtr, TClausePtr>>& doUpdate) override;
    
    // Запросы UPDATE
    std::string BuildUpdate(
        const TMessagePath& table,
        const std::vector<std::pair<TClausePtr, TClausePtr>>& updates, 
        TClausePtr where) override;
    
    // Запросы DELETE
    std::string BuildDelete(const TMessagePath& table, TClausePtr where) override;
    std::string BuildTruncate(TMessagePath path) override;

    // Транзакции
    std::string BuildStartTransaction(bool readOnly) override;
    std::string BuildCommitTransaction() override;
    std::string BuildRollbackTransaction() override;

    // Операции с таблицами
    std::string BuildCreateTable(NOrm::NRelation::TMessageInfoPtr message) override;
    std::string BuildDropTable(NOrm::NRelation::TMessageInfoPtr message) override;
    
    // Операции с колонками
    std::string BuildAddColumn(NOrm::NRelation::TPrimitiveFieldInfoPtr field) override;
    std::string BuildDropColumn(NOrm::NRelation::TPrimitiveFieldInfoPtr field) override;
    std::string BuildAlterColumn(
        NOrm::NRelation::TPrimitiveFieldInfoPtr newField,
        NOrm::NRelation::TPrimitiveFieldInfoPtr oldField) override;
    std::string BuildAlterTable(const std::vector<TClausePtr>& operations) override;
    
    // Объединение запросов
    std::string JoinQueries(const std::vector<std::string>& queries) override;

private:
    // Вспомогательные методы для PostgreSQL
    std::string EscapeIdentifier(const std::string& identifier);
    std::string EscapeStringLiteral(const std::string& str);
    std::string GetPostgresType(const TValueInfo& typeInfo);
    std::string ColumnDefinition(NOrm::NRelation::TPrimitiveFieldInfoPtr field);

    std::vector<std::string> BuildVector(const std::vector<TClausePtr>& cluases);

    StackWrapper<NOrm::NRelation::Builder::EClauseType> Stack_;
};

using TPostgresBuilderPtr = std::shared_ptr<TPostgresBuilder>;

} // namespace NOrm::NRelation::Builder
