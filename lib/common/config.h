#pragma once

#include <common/refcounted.h>
#include <common/intrusive_ptr.h>
#include <common/exception.h>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <optional>

namespace NCommon {

////////////////////////////////////////////////////////////////////////////////

class TConfigBase
    : public NRefCounted::TRefCountedBase
{
public:
    using TJsonData = nlohmann::json;

    void LoadFromFile(const std::filesystem::path& filePath);

    virtual void Load(const TJsonData& data) = 0;

protected:
    template <typename Type>
    requires (!std::is_base_of_v<TConfigBase, Type>)
    static Type Load(const TJsonData& data, const std::string& key, const std::optional<Type>& dflt = {}) {
        return data.value(key, *dflt);
    }

    template <typename Type>
    requires (!std::is_base_of_v<TConfigBase, Type>)
    static Type LoadRequired(const TJsonData& data, const std::string& key) {
        ASSERT(data.contains(key), "Config must require '{}' parameter", key);
        return data.at(key).template get<Type>();
    }

    template <typename Type>
    requires (std::is_base_of_v<TConfigBase, Type>)
    static NCommon::TIntrusivePtr<Type> Load(const TJsonData& data, const std::string& key, const std::optional<Type>& dflt = {}) {
        NCommon::TIntrusivePtr<Type> result = NCommon::New<Type>();
        if (data.contains(key)) {
            result->Load(data.at(key));
        }
        return result;
    }

    template <typename Type>
    requires (std::is_base_of_v<TConfigBase, Type>)
    static NCommon::TIntrusivePtr<Type> LoadRequired(const TJsonData& data, const std::string& key) {
        ASSERT(data.contains(key), "Config must require '{}' parameter", key);
        NCommon::TIntrusivePtr<Type> result = NCommon::New<Type>();
        result->Load(data.at(key));
        return result;
    }

};

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

} // namespace NCommon
