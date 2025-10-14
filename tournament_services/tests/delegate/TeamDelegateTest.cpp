#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "delegate/TeamDelegate.hpp"
#include "persistence/repository/IRepository.hpp"
#include "domain/Team.hpp"
#include <optional>

// 1. Crear un Mock del Repositorio
// NOTA: Asegúrate que la ruta al header IRepository sea correcta
class MockTeamRepository : public IRepository<domain::Team, std::string> {
public:
    MOCK_METHOD(std::optional<std::string>, Create, (const domain::Team& entity), (override));
    MOCK_METHOD(std::shared_ptr<domain::Team>, ReadById, (std::string id), (override));
    MOCK_METHOD(std::vector<std::shared_ptr<domain::Team>>, ReadAll, (), (override));
    MOCK_METHOD(std::string, Update, (const domain::Team& entity), (override));
    MOCK_METHOD(void, Delete, (std::string id), (override));
};

TEST(TeamDelegateTest, SaveTeam_Success) {
    // Arrange
    auto mockRepo = std::make_shared<MockTeamRepository>();
    TeamDelegate delegate(mockRepo);
    domain::Team newTeam{"", "Team A"};
    std::string expectedId = "new-uuid-123";

    // Configurar el mock para simular una creación exitosa en la BD
    EXPECT_CALL(*mockRepo, Create(testing::_))
        .WillOnce(testing::Return(std::optional<std::string>(expectedId)));

    // Act
    auto result = delegate.SaveTeam(newTeam);

    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), expectedId);
}

TEST(TeamDelegateTest, SaveTeam_Conflict) {
    // Arrange
    auto mockRepo = std::make_shared<MockTeamRepository>();
    TeamDelegate delegate(mockRepo);
    domain::Team existingTeam{"", "Team B"};

    // Configurar el mock para simular una inserción fallida (conflicto)
    EXPECT_CALL(*mockRepo, Create(testing::_))
        .WillOnce(testing::Return(std::nullopt));

    // Act
    auto result = delegate.SaveTeam(existingTeam);

    // Assert
    ASSERT_FALSE(result.has_value());
}