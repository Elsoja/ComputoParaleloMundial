#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "delegate/TeamDelegate.hpp"
#include "persistence/repository/IRepository.hpp"
#include "domain/Team.hpp"
#include <optional>
#include <vector>

// Usamos 'using' para simplificar las llamadas a los matchers de gmock
using ::testing::Return;
using ::testing::_; // El comodín para "cualquier argumento"

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

// Prueba de creación exitosa [cite: 64, 65]
TEST(TeamDelegateTest, SaveTeam_Success) {
    // Preparación
    auto mockRepo = std::make_shared<MockTeamRepository>();
    TeamDelegate delegate(mockRepo);
    domain::Team newTeam{"", "Team A"};
    std::string expectedId = "new-uuid-123";

    // Simular que el repositorio devuelve un ID de éxito
    EXPECT_CALL(*mockRepo, Create(newTeam))
        .WillOnce(Return(std::optional<std::string>(expectedId)));

    // Acción
    auto result = delegate.SaveTeam(newTeam);

    // Verificación
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), expectedId);
}

// Prueba de creación fallida (conflicto) [cite: 67, 68]
TEST(TeamDelegateTest, SaveTeam_Conflict) {
    // Preparación
    auto mockRepo = std::make_shared<MockTeamRepository>();
    TeamDelegate delegate(mockRepo);
    domain::Team existingTeam{"", "Team B"};

    // Simular que el repositorio falla y devuelve un optional vacío
    EXPECT_CALL(*mockRepo, Create(existingTeam))
        .WillOnce(Return(std::nullopt));

    // Acción
    auto result = delegate.SaveTeam(existingTeam);

    // Verificación
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), ITeamDelegate::SaveError::Conflict);
}

// --- Pruebas para Búsqueda por ID ---

// Prueba de búsqueda por ID exitosa [cite: 69, 70]
TEST(TeamDelegateTest, GetTeam_ReturnsTeam_WhenFound) {
    // Preparación
    auto mockRepo = std::make_shared<MockTeamRepository>();
    TeamDelegate delegate(mockRepo);
    std::string teamId = "id1";
    auto expectedTeam = std::make_shared<domain::Team>(domain::Team{teamId, "Team One"});

    // Simular que el repositorio encuentra y devuelve el equipo
    EXPECT_CALL(*mockRepo, ReadById(teamId))
        .WillOnce(Return(expectedTeam));

    // Acción
    auto result = delegate.GetTeam(teamId);

    // Verificación
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->Id, teamId);
    ASSERT_EQ(result->Name, "Team One");
}

// Prueba de búsqueda por ID fallida (no encontrado) [cite: 71, 72]
TEST(TeamDelegateTest, GetTeam_ReturnsNull_WhenNotFound) {
    // Preparación
    auto mockRepo = std::make_shared<MockTeamRepository>();
    TeamDelegate delegate(mockRepo);
    
    // Simular que el repositorio no encuentra nada (devuelve nullptr)
    EXPECT_CALL(*mockRepo, ReadById(_))
        .WillOnce(Return(nullptr));

    // Acción
    auto result = delegate.GetTeam("non-existing-id");

    // Verificación
    ASSERT_EQ(result, nullptr);
}

// --- Pruebas para Búsqueda de Todos ---

// Prueba de búsqueda de todos con resultados [cite: 73]
TEST(TeamDelegateTest, GetAllTeams_ReturnsListOfTeams) {
    // Preparación
    auto mockRepo = std::make_shared<MockTeamRepository>();
    TeamDelegate delegate(mockRepo);
    std::vector<std::shared_ptr<domain::Team>> teams = {
        std::make_shared<domain::Team>(domain::Team{"id1", "Team One"})
    };

    // Simular que el repositorio devuelve una lista con un equipo
    EXPECT_CALL(*mockRepo, ReadAll())
        .WillOnce(Return(teams));
    
    // Acción
    auto result = delegate.GetAllTeams();

    // Verificación
    ASSERT_EQ(result.size(), 1);
}

// Prueba de búsqueda de todos con lista vacía [cite: 75]
TEST(TeamDelegateTest, GetAllTeams_ReturnsEmptyList) {
    // Preparación
    auto mockRepo = std::make_shared<MockTeamRepository>();
    TeamDelegate delegate(mockRepo);
    
    // Simular que el repositorio devuelve una lista vacía
    EXPECT_CALL(*mockRepo, ReadAll())
        .WillOnce(Return(std::vector<std::shared_ptr<domain::Team>>()));

    // Acción
    auto result = delegate.GetAllTeams();
    
    // Verificación
    ASSERT_TRUE(result.empty());
}

// --- Pruebas para Actualización ---

// Prueba de actualización exitosa [cite: 76, 77]
TEST(TeamDelegateTest, UpdateTeam_ReturnsSuccess_WhenTeamExists) {
    // Preparación
    auto mockRepo = std::make_shared<MockTeamRepository>();
    TeamDelegate delegate(mockRepo);
    std::string teamId = "existing-id";
    auto existingTeam = std::make_shared<domain::Team>(domain::Team{teamId, "Original Name"});
    domain::Team updatedTeam{teamId, "Updated Name"};

    // Simular que primero se encuentra el equipo y luego se llama a Update
    EXPECT_CALL(*mockRepo, ReadById(teamId))
        .WillOnce(Return(existingTeam));
    EXPECT_CALL(*mockRepo, Update(updatedTeam))
        .WillOnce(Return(teamId));

    // Acción
    auto result = delegate.UpdateTeam(teamId, updatedTeam);

    // Verificación
    ASSERT_TRUE(result.has_value());
}

// Prueba de actualización fallida (no encontrado) [cite: 79, 80]
TEST(TeamDelegateTest, UpdateTeam_ReturnsNotFound_WhenTeamDoesNotExist) {
    // Preparación
    auto mockRepo = std::make_shared<MockTeamRepository>();
    TeamDelegate delegate(mockRepo);
    std::string teamId = "non-existing-id";
    domain::Team updatedTeam{teamId, "Updated Name"};

    // Simular que el equipo a actualizar no es encontrado
    EXPECT_CALL(*mockRepo, ReadById(teamId))
        .WillOnce(Return(nullptr));

    // Acción
    auto result = delegate.UpdateTeam(teamId, updatedTeam);

    // Verificación
    ASSERT_FALSE(result.has_value());
    ASSERT_EQ(result.error(), ITeamDelegate::SaveError::NotFound);
}