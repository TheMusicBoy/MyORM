#pragma once

#include <relation/base.h>
#include <map>

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

class TFieldRange;
class TPrimitiveFieldsRange;
class TMessageFieldsRange;

////////////////////////////////////////////////////////////////////////////////

class TMessageInfo : virtual public TMessageBase {
  public:
    // Method to get range of all fields
    TFieldsRange Fields();

    // Method to get range of primitive fields only
    TPrimitiveFieldsRange PrimitiveFields();

    // Method to get range of message fields only
    TMessageFieldsRange MessageFields();

    virtual std::string GetId() const = 0;

    TMessageInfo(const google::protobuf::Descriptor* descriptor);

    void Process();

  protected:
    std::string MessageName_;
    std::map<int, TFieldBasePtr> Fields_;
    std::map<int, TFieldBasePtr> TablePrimaryFields_;
    std::vector<google::protobuf::FieldDescriptor::Type> IndexFields_;
    std::set<int> SubMessages_;
    const google::protobuf::Descriptor* Descriptor_;
};

using TMessageInfoPtr = std::shared_ptr<TMessageInfo>;

////////////////////////////////////////////////////////////////////////////////

class TFieldMessage : public TFieldBase, public TMessageInfo {
  public:
    TFieldMessage(const google::protobuf::FieldDescriptor* descriptor, const TMessagePath& path)
        : TFieldBase(descriptor, path),
          TMessageInfo(descriptor->message_type()) {}

    bool IsMessage() const override {
        return true;
    }

    std::string GetId() const override {
        return Format("t_{num;name=false;onlydelim;delimiter='_'}", GetPath());
    }
};

using TFieldMessagePtr = std::shared_ptr<TFieldMessage>;

////////////////////////////////////////////////////////////////////////////////

class TMessageFieldIterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = TFieldMessagePtr;
    using difference_type = std::ptrdiff_t;
    using pointer = TFieldMessagePtr*;
    using reference = TFieldMessagePtr&;

    TMessageFieldIterator(
        std::map<int, TFieldBasePtr>::iterator it,
        std::map<int, TFieldBasePtr>::iterator end
    );

    TMessageFieldIterator& operator++();
    TMessageFieldIterator operator++(int);
    bool operator==(const TMessageFieldIterator& other) const;
    bool operator!=(const TMessageFieldIterator& other) const;
    TFieldMessagePtr operator*() const;
    TFieldMessagePtr operator->() const;

  private:
    void skipNonMessageFields();

    std::map<int, TFieldBasePtr>::iterator it_;
    std::map<int, TFieldBasePtr>::iterator end_;
};

class TMessageFieldsRange {
  public:
    TMessageFieldsRange(std::map<int, TFieldBasePtr>& fields);

    TMessageFieldIterator begin();
    TMessageFieldIterator end();

  private:
    std::map<int, TFieldBasePtr>& fields_;
};

////////////////////////////////////////////////////////////////////////////////

class TRootMessage : public TRootBase, public TMessageInfo {
  public:
    TRootMessage(TTableConfigPtr config)
        : TRootBase(config),
          TMessageInfo(GetDescriptor()) {}

    std::string GetId() const override {
        return Format("t_{}", GetPath().number());
    }
};

using TRootMessagePtr = std::shared_ptr<TRootMessage>;

void RegisterRootMessage(TTableConfigPtr config);

////////////////////////////////////////////////////////////////////////////////

} // namespace NOrm::NRelation
