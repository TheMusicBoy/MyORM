#pragma once

#include <query_builder/builder_base.h>
#include <query_builder/query_organizer_base.h>

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

class TSqlQueryOrganizer : public TQueryOrganizerBase {
public:
    TSqlQueryOrganizer();

    Builder::TSelectPtr OrganizeSelect(const TSelect& query) const override;
    Builder::TInsertPtr OrganizeInsert(const TInsert& query) const override;
    Builder::TQueryPtr OrganizeUpdate(const TUpdate& query) const override;
    Builder::TQueryPtr OrganizeDelete(const TDelete& query) const override;
    
    Builder::TQueryPtr CreateTable(const TRootMessage& table) const override;
    Builder::TQueryPtr DeleteTable(const TRootMessage& table) const override;

    Builder::TQueryPtr StartTransaction(const TMessagePath& table) const override;
    Builder::TQueryPtr CommitTransaction(const TMessagePath& table) const override;
    Builder::TQueryPtr RollbackTransaction(const TMessagePath& table) const override;

private:
    Builder::TClausePtr TransformClause(TClause clause) const;

    std::vector<Builder::TClausePtr> ExpandSelector(TClause clause) const;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NOrm::NRelation
