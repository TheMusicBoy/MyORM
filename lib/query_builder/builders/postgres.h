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

protected:
    // Базовые типы данных (protected)
    std::string BuildString(TStringPtr value) override;
    std::string BuildInt(TIntPtr value) override;
    std::string BuildFloat(TFloatPtr value) override;
    std::string BuildBool(TBoolPtr value) override;

    // Выражения и колонки
    std::string BuildExpression(TExpressionPtr expression) override;
    std::string BuildAll(TAllPtr all) override;
    std::string BuildColumn(TColumnPtr column) override;
    std::string BuildTable(TTablePtr table) override;
    std::string BuildDefault(TDefaultPtr defaultVal) override;
    
    std::string BuildJoin(TJoinPtr join) override;

    // Запросы SELECT
    std::string BuildSelect(TSelectPtr select) override;
    
    // Запросы INSERT
    std::string BuildInsert(TInsertPtr insert) override;
    
    // Запросы UPDATE
    std::string BuildUpdate(TUpdatePtr update) override;
    
    // Запросы DELETE
    std::string BuildDelete(TDeletePtr deleteClause) override;
    std::string BuildTruncate(TTruncatePtr truncate) override;

    // Транзакции
    std::string BuildStartTransaction(TStartTransactionPtr startTransaction) override;
    std::string BuildCommitTransaction(TCommitTransactionPtr commitTransaction) override;
    std::string BuildRollbackTransaction(TRollbackTransactionPtr rollbackTransaction) override;

    // Операции с таблицами
    std::string BuildColumnDefinition(TColumnDefinitionPtr columnDefinition) override;
    std::string BuildCreateTable(TCreateTablePtr createTable) override;
    std::string BuildDropTable(TDropTablePtr dropTable) override;
    std::string BuildAlterTable(TAlterTablePtr alterTable) override;
    
    // Операции с колонками
    std::string BuildAddColumn(TAddColumnPtr addColumn) override;
    std::string BuildDropColumn(TDropColumnPtr dropColumn) override;
    std::string BuildAlterColumn(TAlterColumnPtr alterColumn) override;
    
    // Объединение запросов
    std::string JoinQueries(const std::vector<std::string>& queries) override;

private:
    // Вспомогательные методы для PostgreSQL
    std::string EscapeIdentifier(const std::string& identifier);
    std::string EscapeStringLiteral(const std::string& str);
    std::string GetPostgresType(const TValueInfo& typeInfo);
    std::string GetPostgresDefault(const TValueInfo& typeInfo);
    std::string ColumnDefinition(NOrm::NRelation::TPrimitiveFieldInfoPtr field);
    
    // Вспомогательные методы
    std::vector<std::string> BuildVector(const std::vector<TClausePtr>& clauses);

    StackWrapper<NOrm::NRelation::Builder::EClauseType> Stack_;
};

using TPostgresBuilderPtr = std::shared_ptr<TPostgresBuilder>;

} // namespace NOrm::NRelation::Builder
