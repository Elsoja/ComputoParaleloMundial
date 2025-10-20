#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "delegate/TeamDelegate.hpp"
#include "persistence/repository/IRepository.hpp"
#include "domain/Team.hpp"
#include <optional>
#include <vector>

using ::testing::Return;
using ::testing::_;

// Creamos un Mock del Repositorio que simulará todas las operaciones de la BD
class MockTeamRepository : public IRepository<domain::Team, std::string> {
public:
    MOCK_METHOD(std::optional<std::string>, Create, (const domain::Team& entity), (override));
    MOCK_METHOD(std::shared_ptr<domain::Team>, ReadById, (std::string id), (override));
    MOCK_METHOD(std::vector<std::shared_ptr<domain::Team>>, ReadAll, (), (override));
    MOCK_METHOD(std::string, Update, (const domain::Team& entity), (override));
    MOCK_METHOD(void, Delete, (std::string id), (override));
};

// --- Pruebas para Creación ---

TEST(TeamDelegateTest, SaveTeam_Success) {
    auto mockRepo = std::make_shared<MockTeamRepository>();
    TeamDelegate delegate(mockRepo);
    domain::Team newTeam{"", "Team A"};
    std::string expectedId = "new-uuid-123";

    // Simular una creación exitosa en la BD
    EXPECT_CALL(*mockRepo, Create(_))
        .WillOnce(Return(std::optional<std::string>(expectedId)));

    auto result = delegate.SaveTeam(newTeam);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), expectedId);
}

TEST(TeamDelegateTest, SaveTeam_Conflict) {
    auto mockRepo = std::make_shared<MockTeamRepository>();
    TeamDelegate delegate(mockRepo);
    domain::Team existingTeam{"", "Team B"};

    // Simular una inserción fallida (conflicto)
    EXPECT_CALL(*mockRepo, Create(_))
        .WillOnce(Return(std::nullopt));

    auto result = delegate.SaveTeam(existingTeam);

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), ITeamDelegate::SaveError::Conflict);
}

// --- **NUEVO: Pruebas para Búsqueda** ---

TEST(TeamDelegateTest, GetTeam_ReturnsTeam_WhenFound) {
    auto mockRepo = std::make_shared<MockTeamRepository>();
    TeamDelegate delegate(mockRepo);
    std::string teamId = "id1";
    auto expectedTeam = std::make_shared<domain::Team>(domain::Team{teamId, "Team One"});

    // Simular que el repositorio encuentra y devuelve el equipo
    EXPECT_CALL(*mockRepo, ReadById(teamId))
        .WillOnce(Return(expectedTeam));

    auto result = delegate.GetTeam(teamId);

    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->Id, teamId);
}

TEST(TeamDelegateTest, GetTeam_ReturnsNull_WhenNotFound) {
    auto mockRepo = std::make_shared<MockTeamRepository>();
    TeamDelegate delegate(mockRepo);
    
    // Simular que el repositorio no encuentra nada
    EXPECT_CALL(*mockRepo, ReadById(_))
        .WillOnce(Return(nullptr));

    auto result = delegate.GetTeam("non-existing-id");

    ASSERT_EQ(result, nullptr);
}

TEST(TeamDelegateTest, GetAllTeams_ReturnsListOfTeams) {
    auto mockRepo = std::make_shared<MockTeamRepository>();
    TeamDelegate delegate(mockRepo);
    std::vector<std::shared_ptr<domain::Team>> teams = {
        std::make_shared<domain::Team>(domain::Team{"id1", "Team One"})
    };

    // Simular que el repositorio devuelve una lista con un equipo
    EXPECT_CALL(*mockRepo, ReadAll())
        .WillOnce(Return(teams));
    
    auto result = delegate.GetAllTeams();

    ASSERT_EQ(result.size(), 1);
}

TEST(TeamDelegateTest, GetAllTeams_ReturnsEmptyList) {
    auto mockRepo = std::make_shared<MockTeamRepository>();
    TeamDelegate delegate(mockRepo);
    
    // Simular que el repositorio devuelve una lista vacía
    EXPECT_CALL(*mockRepo, ReadAll())
        .WillOnce(Return(std::vector<std::shared_ptr<domain::Team>>()));

    auto result = delegate.GetAllTeams();
    
    ASSERT_TRUE(result.empty());
}

// --- **NUEVO: Pruebas para Actualización** ---

TEST(TeamDelegateTest, UpdateTeam_ReturnsSuccess_WhenTeamExists) {
    auto mockRepo = std::make_shared<MockTeamRepository>();
    TeamDelegate delegate(mockRepo);
    std::string teamId = "existing-id";
    auto existingTeam = std::make_shared<domain::Team>(domain::Team{teamId, "Original Name"});
    domain::Team updatedTeam{teamId, "Updated Name"};

    // Simular que el equipo es encontrado, y luego la actualización es exitosa
    EXPECT_CALL(*mockRepo, ReadById(teamId))
        .WillOnce(Return(existingTeam));
    EXPECT_CALL(*mockRepo, Update(updatedTeam))
        .WillOnce(Return(teamId)); // Simula que Update devuelve el ID

    auto result = delegate.UpdateTeam(teamId, updatedTeam);

    ASSERT_TRUE(result.has_value());
}

TEST(TeamDelegateTest, UpdateTeam_ReturnsNotFound_WhenTeamDoesNotExist) {
    auto mockRepo = std::make_shared<MockTeamRepository>();
    TeamDelegate delegate(mockRepo);
    std::string teamId = "non-existing-id";
    domain::Team updatedTeam{teamId, "Updated Name"};

    // Simular que el equipo no es encontrado
    EXPECT_CALL(*mockRepo, ReadById(teamId))
        .WillOnce(Return(nullptr));

    auto result = delegate.UpdateTeam(teamId, updatedTeam);

    ASSERT_FALSE(result.has_value());
    ASSERT_EQ(result.error(), ITeamDelegate::SaveError::NotFound);
}