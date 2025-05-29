#pragma once

#include <common/config.h>

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

class TTableConfig
    : public ::NCommon::TConfigBase
{
public:
    int Number;
    std::string SnakeCase;
    std::string CamelCase;

    std::string Scheme;

    void Load(const TJsonData& data) override;
};

DECLARE_REFCOUNTED(TTableConfig);

////////////////////////////////////////////////////////////////////////////////

class TOrmConfig
    : public ::NCommon::TConfigBase
{
public:
    std::vector<TTableConfigPtr> Tables;

    void Load(const TJsonData& data) override;
};

DECLARE_REFCOUNTED(TOrmConfig);

////////////////////////////////////////////////////////////////////////////////

} // namespace NOrm::NRelation
