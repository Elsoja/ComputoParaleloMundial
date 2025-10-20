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

// --- Mocks Necesarios ---

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

// Prueba creación exitosa [cite: 261, 262]
TEST(GroupDelegateTest, CreateGroup_Success_WhenTournamentExists) {
    auto mockTournRepo = std::make_shared<MockTournamentRepository>();
    auto mockGroupRepo = std::make_shared<MockGroupRepository>();
    auto mockTeamRepo = std::make_shared<MockTeamRepository>();
    GroupDelegate delegate(mockTournRepo, mockGroupRepo, mockTeamRepo);

    std::string tournamentId = "tourn-1";
    auto tournament = std::make_shared<domain::Tournament>("Existing Tournament");
    tournament->Id() = tournamentId; // Importante asignar el ID
    domain::Group newGroup("Group Alpha");
    std::string expectedGroupId = "group-uuid-789";

    // Simular que el torneo es encontrado
    EXPECT_CALL(*mockTournRepo, ReadById(tournamentId))
        .WillOnce(Return(tournament));
    // Simular que la creación del grupo en su repositorio es exitosa
    EXPECT_CALL(*mockGroupRepo, Create(_)) // No validamos el objeto exacto aquí, solo que se llame
        .WillOnce(Return(std::optional<std::string>(expectedGroupId)));

    // Acción
    auto result = delegate.CreateGroup(tournamentId, newGroup);

    // Verificación
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), expectedGroupId);
}

// Prueba creación fallida - Torneo no encontrado
TEST(GroupDelegateTest, CreateGroup_Fails_WhenTournamentNotFound) {
    // Preparación
    auto mockTournRepo = std::make_shared<MockTournamentRepository>();
    auto mockGroupRepo = std::make_shared<MockGroupRepository>();
    auto mockTeamRepo = std::make_shared<MockTeamRepository>();
    GroupDelegate delegate(mockTournRepo, mockGroupRepo, mockTeamRepo);

    std::string tournamentId = "non-existing-tourn";
    domain::Group newGroup("Group Beta");

    // Simular que el torneo NO es encontrado
    EXPECT_CALL(*mockTournRepo, ReadById(tournamentId))
        .WillOnce(Return(nullptr));
    // El repositorio de grupos NO debe ser llamado si el torneo no existe
    EXPECT_CALL(*mockGroupRepo, Create(_)).Times(0);

    // Acción
    auto result = delegate.CreateGroup(tournamentId, newGroup);

    // Verificación
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "Tournament not found");
}

// Prueba creación fallida - Conflicto en GroupRepository [cite: 263, 264]
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
    // Simular que el GroupRepository falla (devuelve optional vacío)
    EXPECT_CALL(*mockGroupRepo, Create(_))
        .WillOnce(Return(std::nullopt));

    // Acción
    auto result = delegate.CreateGroup(tournamentId, existingGroup);

    // Verificación
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "Group could not be created, possibly a duplicate name.");
}

// --- Pruebas para Búsqueda ---

// Prueba búsqueda de todos los grupos de un torneo [cite: 268] (Asumiendo que GetGroups busca en ReadAll)
TEST(GroupDelegateTest, GetGroups_ReturnsCorrectGroupsForTournament) {
    // Preparación
    auto mockGroupRepo = std::make_shared<MockGroupRepository>();
    // Los otros mocks no son estrictamente necesarios aquí si GetGroups solo usa GroupRepository
    auto mockTournRepo = std::make_shared<MockTournamentRepository>();
    auto mockTeamRepo = std::make_shared<MockTeamRepository>();
    GroupDelegate delegate(mockTournRepo, mockGroupRepo, mockTeamRepo);

    std::string tournamentId = "tourn-1";
    auto group1 = std::make_shared<domain::Group>("Group A");
    group1->Id() = "g1"; group1->TournamentId() = tournamentId;
    auto group2 = std::make_shared<domain::Group>("Group B"); // Pertenece a otro torneo
    group2->Id() = "g2"; group2->TournamentId() = "tourn-2";
    auto group3 = std::make_shared<domain::Group>("Group C");
    group3->Id() = "g3"; group3->TournamentId() = tournamentId;

    std::vector<std::shared_ptr<domain::Group>> allGroups = { group1, group2, group3 };

    // Simular que ReadAll devuelve todos los grupos
    EXPECT_CALL(*mockGroupRepo, ReadAll())
        .WillOnce(Return(allGroups));

    // Acción
    auto result = delegate.GetGroups(tournamentId);

    // Verificación
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result.value().size(), 2); // Solo debe devolver los grupos g1 y g3
    EXPECT_EQ(result.value()[0].Name(), "Group A");
    EXPECT_EQ(result.value()[1].Name(), "Group C");
}

// Prueba búsqueda de un grupo por ID - Éxito [cite: 268, 269]
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
    EXPECT_CALL(*mockGroupRepo, ReadById(groupId))
        .WillOnce(Return(expectedGroup));

    // Acción
    auto result = delegate.GetGroup(tournamentId, groupId);

    // Verificación
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().Id(), groupId);
    EXPECT_EQ(result.value().TournamentId(), tournamentId);
}

// Prueba búsqueda de un grupo por ID - No encontrado [cite: 270, 271]
TEST(GroupDelegateTest, GetGroup_ReturnsError_WhenNotFound) {
    // Preparación
    auto mockGroupRepo = std::make_shared<MockGroupRepository>();
    auto mockTournRepo = std::make_shared<MockTournamentRepository>();
    auto mockTeamRepo = std::make_shared<MockTeamRepository>();
    GroupDelegate delegate(mockTournRepo, mockGroupRepo, mockTeamRepo);

    std::string tournamentId = "tourn-1";
    std::string groupId = "non-existing-group";

    // Simular que el repositorio NO encuentra el grupo
    EXPECT_CALL(*mockGroupRepo, ReadById(groupId))
        .WillOnce(Return(nullptr));

    // Acción
    auto result = delegate.GetGroup(tournamentId, groupId);

    // Verificación
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "Group not found in this tournament");
}

// Prueba búsqueda de un grupo por ID - Pertenece a otro torneo
TEST(GroupDelegateTest, GetGroup_ReturnsError_WhenFoundInDifferentTournament) {
    // Preparación
    auto mockGroupRepo = std::make_shared<MockGroupRepository>();
    auto mockTournRepo = std::make_shared<MockTournamentRepository>();
    auto mockTeamRepo = std::make_shared<MockTeamRepository>();
    GroupDelegate delegate(mockTournRepo, mockGroupRepo, mockTeamRepo);

    std::string targetTournamentId = "tourn-1";
    std::string wrongTournamentId = "tourn-2";
    std::string groupId = "g1";
    auto foundGroup = std::make_shared<domain::Group>("Group A");
    foundGroup->Id() = groupId;
    foundGroup->TournamentId() = wrongTournamentId; // Pertenece a otro torneo

    // Simular que el repositorio encuentra el grupo, pero con el ID de torneo incorrecto
    EXPECT_CALL(*mockGroupRepo, ReadById(groupId))
        .WillOnce(Return(foundGroup));

    // Acción
    auto result = delegate.GetGroup(targetTournamentId, groupId);

    // Verificación
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "Group not found in this tournament");
}

// --- Pruebas para Actualización ---

// Prueba actualización exitosa [cite: 272, 273]
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
    updatedGroup.TournamentId() = tournamentId; // Importante para la validación interna si la hubiera

    // Simular que el grupo es encontrado
    EXPECT_CALL(*mockGroupRepo, ReadById(groupId))
        .WillOnce(Return(existingGroup));
    // Simular que la actualización es exitosa
    EXPECT_CALL(*mockGroupRepo, Update(updatedGroup))
        .WillOnce(Return(groupId)); // Update devuelve el ID

    // Acción
    auto result = delegate.UpdateGroup(tournamentId, updatedGroup);

    // Verificación
    ASSERT_TRUE(result.has_value());
}

// Prueba actualización fallida - No encontrado [cite: 276, 277]
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
    EXPECT_CALL(*mockGroupRepo, ReadById(groupId))
        .WillOnce(Return(nullptr));

    // Acción
    auto result = delegate.UpdateGroup(tournamentId, updatedGroup);

    // Verificación
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "Group not found in this tournament");
}

// --- Pruebas para Borrado --- (Implícito en el CRUD, aunque no listado explícitamente)

TEST(GroupDelegateTest, RemoveGroup_Success_WhenFoundInTournament) {
    // Preparación
    auto mockGroupRepo = std::make_shared<MockGroupRepository>();
    auto mockTournRepo = std::make_shared<MockTournamentRepository>();
    auto mockTeamRepo = std::make_shared<MockTeamRepository>();
    GroupDelegate delegate(mockTournRepo, mockGroupRepo, mockTeamRepo);

    std::string tournamentId = "tourn-1";
    std::string groupId = "g1";
    auto existingGroup = std::make_shared<domain::Group>("Group to Delete");
    existingGroup->Id() = groupId;
    existingGroup->TournamentId() = tournamentId;

    // Simular que el grupo es encontrado
    EXPECT_CALL(*mockGroupRepo, ReadById(groupId))
        .WillOnce(Return(existingGroup));
    // Esperar que se llame a Delete
    EXPECT_CALL(*mockGroupRepo, Delete(groupId));

    // Acción
    auto result = delegate.RemoveGroup(tournamentId, groupId);

    // Verificación
    ASSERT_TRUE(result.has_value());
}

TEST(GroupDelegateTest, RemoveGroup_ReturnsError_WhenNotFound) {
    // Preparación
    auto mockGroupRepo = std::make_shared<MockGroupRepository>();
    auto mockTournRepo = std::make_shared<MockTournamentRepository>();
    auto mockTeamRepo = std::make_shared<MockTeamRepository>();
    GroupDelegate delegate(mockTournRepo, mockGroupRepo, mockTeamRepo);

    std::string tournamentId = "tourn-1";
    std::string groupId = "non-existing-group";

    // Simular que el grupo NO es encontrado
    EXPECT_CALL(*mockGroupRepo, ReadById(groupId))
        .WillOnce(Return(nullptr));

    // Acción
    auto result = delegate.RemoveGroup(tournamentId, groupId);

    // Verificación
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "Group not found in this tournament");
}

// --- Pruebas para Añadir Equipo (No implementado en GroupDelegate, pero requerido por PDF) ---
// Nota: Las pruebas [cite: 279-283] requerirían un método como `AddTeamToGroup` en tu GroupDelegate.
// Como ese método no existe actualmente en tu código, estas pruebas no se pueden implementar aún.
// Si añades esa funcionalidad, el patrón sería similar a las pruebas de Update.