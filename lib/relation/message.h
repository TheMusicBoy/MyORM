#pragma once

#include <relation/base.h>
#include <relation/field.h>
#include <unordered_map>
#include <unordered_set>

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

class TFieldMessage;

class TMessageInfo : virtual public TMessageBase {
public:
    class TPrimitiveFieldIterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = TPrimitiveFieldInfo*;
        using difference_type = std::ptrdiff_t;
        using pointer = TPrimitiveFieldInfo**;
        using reference = TPrimitiveFieldInfo*&;

        TPrimitiveFieldIterator(
            std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator it,
            std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator end,
            const std::unordered_set<int>& subMessages
        );

        TPrimitiveFieldIterator& operator++();
        TPrimitiveFieldIterator operator++(int);
        bool operator==(const TPrimitiveFieldIterator& other) const;
        bool operator!=(const TPrimitiveFieldIterator& other) const;
        TPrimitiveFieldInfo* operator*() const;
        TPrimitiveFieldInfo* operator->() const;

    private:
        void skipMessageFields();

        std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator it_;
        std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator end_;
        const std::unordered_set<int>& subMessages_;
    };

    class TMessageFieldIterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = TFieldMessage*;
        using difference_type = std::ptrdiff_t;
        using pointer = TFieldMessage**;
        using reference = TFieldMessage*&;

        TMessageFieldIterator(
            std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator it,
            std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator end,
            const std::unordered_set<int>& subMessages
        );

        TMessageFieldIterator& operator++();
        TMessageFieldIterator operator++(int);
        bool operator==(const TMessageFieldIterator& other) const;
        bool operator!=(const TMessageFieldIterator& other) const;
        TFieldMessage* operator*() const;
        TFieldMessage* operator->() const;

    private:
        void skipNonMessageFields();

        std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator it_;
        std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator end_;
        const std::unordered_set<int>& subMessages_;
    };

    class TFieldIterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = TFieldBase*;
        using difference_type = std::ptrdiff_t;
        using pointer = TFieldBase**;
        using reference = TFieldBase*&;

        TFieldIterator(std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator it);

        TFieldIterator& operator++();
        TFieldIterator operator++(int);
        bool operator==(const TFieldIterator& other) const;
        bool operator!=(const TFieldIterator& other) const;
        TFieldBase* operator*() const;
        TFieldBase* operator->() const;

    private:
        std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator it_;
    };

    class TPrimitiveFieldsRange {
    public:
        TPrimitiveFieldsRange(
            std::unordered_map<int, std::unique_ptr<TFieldBase>>& fields,
            const std::unordered_set<int>& subMessages
        );

        TPrimitiveFieldIterator begin();
        TPrimitiveFieldIterator end();

    private:
        std::unordered_map<int, std::unique_ptr<TFieldBase>>& fields_;
        const std::unordered_set<int>& subMessages_;
    };

    class TMessageFieldsRange {
    public:
        TMessageFieldsRange(
            std::unordered_map<int, std::unique_ptr<TFieldBase>>& fields,
            const std::unordered_set<int>& subMessages
        );

        TMessageFieldIterator begin();
        TMessageFieldIterator end();

    private:
        std::unordered_map<int, std::unique_ptr<TFieldBase>>& fields_;
        const std::unordered_set<int>& subMessages_;
    };

    class TFieldsRange {
    public:
        TFieldsRange(std::unordered_map<int, std::unique_ptr<TFieldBase>>& fields);

        TFieldIterator begin();
        TFieldIterator end();

    private:
        std::unordered_map<int, std::unique_ptr<TFieldBase>>& fields_;
    };

    // Метод для получения диапазона всех полей
    TFieldsRange Fields();

    // Метод для получения диапазона только примитивных полей
    TPrimitiveFieldsRange PrimitiveFields();

    // Метод для получения диапазона только полей-сообщений
    TMessageFieldsRange MessageFields();

    TMessageInfo(const google::protobuf::Descriptor* descriptor);

protected:
    void Process();

    std::string MessageName_;
    std::unordered_map<int, std::unique_ptr<TFieldBase>> Fields_;
    std::unordered_set<int> SubMessages_;
    const google::protobuf::Descriptor* Descriptor_;
};

////////////////////////////////////////////////////////////////////////////////

class TFieldMessage : public TFieldBase, public TMessageInfo {
  public:
    TFieldMessage(const google::protobuf::FieldDescriptor* descriptor, const TMessagePath& path)
        : TFieldBase(descriptor, path),
          TMessageInfo(descriptor->message_type()) {}
};

////////////////////////////////////////////////////////////////////////////////

class TRootMessage : public TRootBase, public TMessageInfo {
  public:
    TRootMessage(TTableConfigPtr config)
        : TRootBase(config),
          TMessageInfo(GetDescriptor()) {}
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NOrm::NRelation
