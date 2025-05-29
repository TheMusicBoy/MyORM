#pragma once

#include <relation/base.h>
#include <unordered_map>
#include <unordered_set>

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

class TFieldMessage;
class TFieldRange;
class TPrimitiveFieldsRange;
class TMessageFieldsRange;

////////////////////////////////////////////////////////////////////////////////

class TMessageInfo : virtual public TMessageBase {
  public:
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

class TMessageFieldIterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = TFieldMessage*;
    using difference_type = std::ptrdiff_t;
    using pointer = TFieldMessage**;
    using reference = TFieldMessage*&;

    TMessageFieldIterator(
        std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator it,
        std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator end
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
};

class TMessageFieldsRange {
  public:
    TMessageFieldsRange(std::unordered_map<int, std::unique_ptr<TFieldBase>>& fields);

    TMessageFieldIterator begin();
    TMessageFieldIterator end();

  private:
    std::unordered_map<int, std::unique_ptr<TFieldBase>>& fields_;
};

////////////////////////////////////////////////////////////////////////////////

class TFieldMessage : public TFieldBase, public TMessageInfo {
  public:
    TFieldMessage(const google::protobuf::FieldDescriptor* descriptor, const TMessagePath& path)
        : TFieldBase(descriptor, path),
          TMessageInfo(descriptor->message_type()) {}

    bool IsMessage() const override {
        return true;
    }
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
