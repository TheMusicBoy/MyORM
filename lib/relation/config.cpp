#include <relation/config.h>

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

void TTableConfig::Load(const TJsonData& data) {
    Number = NCommon::TConfigBase::LoadRequired<int>(data, "table_number");
    SnakeCase = NCommon::TConfigBase::LoadRequired<std::string>(data, "snake_case");
    CamelCase = NCommon::TConfigBase::LoadRequired<std::string>(data, "camel_case");

    Scheme = NCommon::TConfigBase::LoadRequired<std::string>(data, "scheme");
    CustomTypeHandler = NCommon::TConfigBase::Load<bool>(data, "custom_type_handler", false);

}

void TOrmConfig::Load(const TJsonData& data) {
    for (const auto& table : data.at("tables")) {
        auto config = NCommon::New<TTableConfig>();
        Tables.emplace_back((config));
        config->Load(std::move(table));
    }
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NOrm::NRelation
