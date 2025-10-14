#ifndef TOURNAMENTS_GROUPREPOSITORY_HPP
#define TOURNAMENTS_GROUPREPOSITORY_HPP

#include <string>
#include <memory>
#include <vector>
#include <optional>
#include "persistence/repository/IRepository.hpp"
#include "persistence/configuration/IDbConnectionProvider.hpp"

// Forward declaration para no necesitar el include completo aquí
namespace domain {
    class Group;
}

class GroupRepository : public IRepository<domain::Group, std::string> {
    std::shared_ptr<IDbConnectionProvider> connectionProvider;
public:
    explicit GroupRepository(std::shared_ptr<IDbConnectionProvider> provider);

    std::optional<std::string> Create(const domain::Group & entity) override;
    std::shared_ptr<domain::Group> ReadById(std::string id) override;
    std::string Update(const domain::Group & entity) override;
    void Delete(std::string id) override;
    std::vector<std::shared_ptr<domain::Group>> ReadAll() override;
};

#endif //TOURNAMENTS_GROUPREPOSITORY_HPP