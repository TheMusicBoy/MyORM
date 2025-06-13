#include <query_builder/organizers/sql_organizer.h>

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

TSqlQueryOrganizer::TSqlQueryOrganizer() = default;

Builder::TQueryPtr TSqlQueryOrganizer::OrganizeSelect(const TSelect& query) const {
    Builder::TQueryPtr result = std::make_shared<Builder::TQuery>();

    

    return result;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NOrm::NRelation
