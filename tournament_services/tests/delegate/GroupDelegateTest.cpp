#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "delegate/GroupDelegate.hpp"
#include "persistence/repository/IRepository.hpp"
#include "domain/Group.hpp"
#include "domain/Tournament.hpp"
#include "domain/Team.hpp"
#include <optional>
#include <vector>
#include <memory>
#include <string>

// Usamos 'using' para simplificar las llamadas
using ::testing::Return;
using ::testing::_;
using ::testing::Eq; // Para comparar objetos

// Mock del Repositorio de Grupos
class MockGroupRepository : public IRepository<domain::Group, std::string> {
public:
    MOCK_METHOD(std::optional<std::string>, Create, (const domain::Group& entity), (override));
    MOCK_METHOD(std::shared_ptr<domain::Group>, ReadById, (std::string id), (override));
    MOCK_METHOD(std::vector<std::shared_ptr<domain::Group>>, ReadAll, (), (override));
    MOCK_METHOD(std::string, Update, (const domain::Group& entity), (override));
    MOCK_METHOD(void, Delete, (std::string id), (override));
};

// Mock del Repositorio de Torneos
class MockTournamentRepository : public IRepository<domain::Tournament, std::string> {
public:
    MOCK_METHOD(std::optional<std::string>, Create, (const domain::Tournament& entity), (override));
    MOCK_METHOD(std::shared_ptr<domain::Tournament>, ReadById, (std::string id), (override));
    MOCK_METHOD(std::vector<std::shared_ptr<domain::Tournament>>, ReadAll, (), (override));
    MOCK_METHOD(std::string, Update, (const domain::Tournament& entity), (override));
    MOCK_METHOD(void, Delete, (std::string id), (override));
};

// Mock del Repositorio de Equipos
class MockTeamRepository : public IRepository<domain::Team, std::string> {
public:
    MOCK_METHOD(std::optional<std::string>, Create, (const domain::Team& entity), (override));
    MOCK_METHOD(std::shared_ptr<domain::Team>, ReadById, (std::string id), (override));
    MOCK_METHOD(std::vector<std::shared_ptr<domain::Team>>, ReadAll, (), (override));
    MOCK_METHOD(std::string, Update, (const domain::Team& entity), (override));
    MOCK_METHOD(void, Delete, (std::string id), (override));
};

// --- Pruebas para Creación de Grupo ---

// [cite: 261, 262]
TEST(GroupDelegateTest, CreateGroup_Success_WhenTournamentExists) {
    // Preparación
    auto mockTournRepo = std::make_shared<MockTournamentRepository>();
    auto mockGroupRepo = std::make_shared<MockGroupRepository>();
    auto mockTeamRepo = std::make_shared<MockTeamRepository>();
    GroupDelegate delegate(mockTournRepo, mockGroupRepo, mockTeamRepo);

    std::string tournamentId = "tourn-1";
    auto tournament = std::make_shared<domain::Tournament>("Existing Tournament");
    tournament->Id() = tournamentId;
    domain::Group newGroup("Group Alpha");
    std::string expectedGroupId = "group-uuid-789";

    // Simular que el torneo es encontrado
    EXPECT_CALL(*mockTournRepo, ReadById(tournamentId))
        .WillOnce(Return(tournament));

    // Validar que el Group transferido a GroupRepository tenga el TournamentId correcto
    domain::Group expectedGroupToCreate = newGroup;
    expectedGroupToCreate.TournamentId() = tournamentId;
    EXPECT_CALL(*mockGroupRepo, Create(Eq(expectedGroupToCreate)))
        .WillOnce(Return(std::optional<std::string>(expectedGroupId)));

    // Acción
    auto result = delegate.CreateGroup(tournamentId, newGroup);

    // Verificación
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), expectedGroupId); // Validar ID del grupo
    // Nota: La validación de evento generado requeriría mockear el MessageProducer.
}

// [cite: 263, 264]
TEST(GroupDelegateTest, CreateGroup_Fails_WhenGroupRepoFails) {
    // Preparación
    auto mockTournRepo = std::make_shared<MockTournamentRepository>();
    auto mockGroupRepo = std::make_shared<MockGroupRepository>();
    auto mockTeamRepo = std::make_shared<MockTeamRepository>();
    GroupDelegate delegate(mockTournRepo, mockGroupRepo, mockTeamRepo);

    std::string tournamentId = "tourn-1";
    auto tournament = std::make_shared<domain::Tournament>("Existing Tournament");
    tournament->Id() = tournamentId;
    domain::Group existingGroup("Existing Group Name");

    // Simular que el torneo sí existe
    EXPECT_CALL(*mockTournRepo, ReadById(tournamentId))
        .WillOnce(Return(tournament));
    // Simular que el GroupRepository falla
    EXPECT_CALL(*mockGroupRepo, Create(_))
        .WillOnce(Return(std::nullopt));

    // Acción
    auto result = delegate.CreateGroup(tournamentId, existingGroup);

    // Verificación
    ASSERT_FALSE(result.has_value()); // Regresar error usando expected
    EXPECT_EQ(result.error(), "Group could not be created, possibly a duplicate name.");
}

// --- Pruebas para Búsqueda ---

// [cite: 268, 269]
TEST(GroupDelegateTest, GetGroup_ReturnsGroup_WhenFoundInTournament) {
    // Preparación
    auto mockGroupRepo = std::make_shared<MockGroupRepository>();
    auto mockTournRepo = std::make_shared<MockTournamentRepository>();
    auto mockTeamRepo = std::make_shared<MockTeamRepository>();
    GroupDelegate delegate(mockTournRepo, mockGroupRepo, mockTeamRepo);

    std::string tournamentId = "tourn-1";
    std::string groupId = "g1";
    auto expectedGroup = std::make_shared<domain::Group>("Group A");
    expectedGroup->Id() = groupId;
    expectedGroup->TournamentId() = tournamentId;

    // Simular que el repositorio encuentra el grupo
    EXPECT_CALL(*mockGroupRepo, ReadById(groupId)) // Validar valor transferido
        .WillOnce(Return(expectedGroup));

    // Acción
    auto result = delegate.GetGroup(tournamentId, groupId);

    // Verificación
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().Id(), groupId); // Simular resultado con objeto
    EXPECT_EQ(result.value().TournamentId(), tournamentId);
}

// [cite: 270, 271]
TEST(GroupDelegateTest, GetGroup_ReturnsError_WhenNotFound) {
    // Preparación
    auto mockGroupRepo = std::make_shared<MockGroupRepository>();
    auto mockTournRepo = std::make_shared<MockTournamentRepository>();
    auto mockTeamRepo = std::make_shared<MockTeamRepository>();
    GroupDelegate delegate(mockTournRepo, mockGroupRepo, mockTeamRepo);

    std::string tournamentId = "tourn-1";
    std::string groupId = "non-existing-group";

    // Simular que el repositorio NO encuentra el grupo
    EXPECT_CALL(*mockGroupRepo, ReadById(groupId)) // Validar valor transferido
        .WillOnce(Return(nullptr)); // Simular resultado nulo

    // Acción
    auto result = delegate.GetGroup(tournamentId, groupId);

    // Verificación
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "Group not found in this tournament");
}

// --- Pruebas para Actualización ---

// [cite: 272, 273]
TEST(GroupDelegateTest, UpdateGroup_Success_WhenFoundInTournament) {
    // Preparación
    auto mockGroupRepo = std::make_shared<MockGroupRepository>();
    auto mockTournRepo = std::make_shared<MockTournamentRepository>();
    auto mockTeamRepo = std::make_shared<MockTeamRepository>();
    GroupDelegate delegate(mockTournRepo, mockGroupRepo, mockTeamRepo);

    std::string tournamentId = "tourn-1";
    std::string groupId = "g1";
    auto existingGroup = std::make_shared<domain::Group>("Original Name");
    existingGroup->Id() = groupId;
    existingGroup->TournamentId() = tournamentId;
    domain::Group updatedGroup("Updated Name");
    updatedGroup.Id() = groupId;
    updatedGroup.TournamentId() = tournamentId;

    // Simular que el grupo es encontrado
    EXPECT_CALL(*mockGroupRepo, ReadById(groupId))
        .WillOnce(Return(existingGroup));
    // Validar valor transferido a Update y simular éxito
    EXPECT_CALL(*mockGroupRepo, Update(Eq(updatedGroup)))
        .WillOnce(Return(groupId));

    // Acción
    auto result = delegate.UpdateGroup(tournamentId, updatedGroup);

    // Verificación
    ASSERT_TRUE(result.has_value()); // Simular actualización válida
}

// [cite: 276, 277]
TEST(GroupDelegateTest, UpdateGroup_ReturnsError_WhenNotFound) {
    // Preparación
    auto mockGroupRepo = std::make_shared<MockGroupRepository>();
    auto mockTournRepo = std::make_shared<MockTournamentRepository>();
    auto mockTeamRepo = std::make_shared<MockTeamRepository>();
    GroupDelegate delegate(mockTournRepo, mockGroupRepo, mockTeamRepo);

    std::string tournamentId = "tourn-1";
    std::string groupId = "non-existing-group";
    domain::Group updatedGroup("Updated Name");
    updatedGroup.Id() = groupId;

    // Simular que el grupo NO es encontrado
    EXPECT_CALL(*mockGroupRepo, ReadById(groupId)) // Validar búsqueda
        .WillOnce(Return(nullptr)); // Simular ID no encontrado
    // Update no debe ser llamado
    EXPECT_CALL(*mockGroupRepo, Update(_)).Times(0);

    // Acción
    auto result = delegate.UpdateGroup(tournamentId, updatedGroup);

    // Verificación
    ASSERT_FALSE(result.has_value()); // Regresar valor usando expected
    EXPECT_EQ(result.error(), "Group not found in this tournament");
}