#pragma once

#include <requests/query.h>
#include <relation/message.h>
#include <query_builder/builder_base.h>

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

class TQueryOrganizerBase {
public:
    virtual Builder::TSelectPtr OrganizeSelect(const TSelect& query) const = 0;
    virtual Builder::TQueryPtr OrganizeInsert(const TInsert& query) const = 0;
    virtual Builder::TQueryPtr OrganizeUpdate(const TUpdate& query) const = 0;
    virtual Builder::TQueryPtr OrganizeDelete(const TDelete& query) const = 0;
    
    virtual Builder::TQueryPtr CreateTable(const TRootMessage& table) const = 0;
    virtual Builder::TQueryPtr DeleteTable(const TRootMessage& table) const = 0;

    virtual Builder::TQueryPtr StartTransaction(const TMessagePath& table) const = 0;
    virtual Builder::TQueryPtr CommitTransaction(const TMessagePath& table) const = 0;
    virtual Builder::TQueryPtr RollbackTransaction(const TMessagePath& table) const = 0;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NOrm::NRelation
