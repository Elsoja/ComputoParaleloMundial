#ifndef DATABASE_CONFIGURATION_HPP
#define DATABASE_CONFIGURATION_HPP

#include <string>

namespace configuration {

class DatabaseConfiguration {
private:
    std::string connectionString;

public:
    explicit DatabaseConfiguration(const std::string& connStr)
        : connectionString(connStr) {}

    std::string GetConnectionString() const {
        return connectionString;
    }
};

} // namespace configuration

#endif // DATABASE_CONFIGURATION_HPP