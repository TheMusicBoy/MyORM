#include <query_builder/organizers/sql_organizer.h>

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

TSqlQueryOrganizer::TSqlQueryOrganizer() = default;

Builder::TSelectPtr TSqlQueryOrganizer::OrganizeSelect(const TSelect& query) const {
    Builder::TSelectPtr result = std::make_shared<Builder::TSelect>();

    

    return result;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NOrm::NRelation
