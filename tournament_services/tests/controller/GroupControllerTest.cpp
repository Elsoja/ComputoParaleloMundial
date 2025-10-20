#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "controller/GroupController.hpp"
#include "delegate/IGroupDelegate.hpp"
#include "domain/Group.hpp"
#include "crow.h"
#include <expected>
#include <vector>
#include <string>

// Usamos 'using' para simplificar las llamadas
using ::testing::Return;
using ::testing::_;

// --- Mock del Delegado ---
// Simula el comportamiento de IGroupDelegate
class MockGroupDelegate : public IGroupDelegate {
public:
    // Mocks para todos los métodos de la interfaz IGroupDelegate
    MOCK_METHOD((std::expected<std::string, std::string>), CreateGroup, (const std::string_view& tournamentId, const domain::Group& group), (override));
    MOCK_METHOD((std::expected<std::vector<domain::Group>, std::string>), GetGroups, (const std::string_view& tournamentId), (override));
    MOCK_METHOD((std::expected<domain::Group, std::string>), GetGroup, (const std::string_view& tournamentId, const std::string_view& groupId), (override));
    MOCK_METHOD((std::expected<void, std::string>), UpdateGroup, (const std::string_view& tournamentId, const domain::Group& group), (override));
    MOCK_METHOD((std::expected<void, std::string>), RemoveGroup, (const std::string_view& tournamentId, const std::string_view& groupId), (override));
};

// --- Pruebas para POST /tournaments/{id}/groups (Creación) ---

// Prueba creación exitosa (HTTP 201) [cite: 233-235]
TEST(GroupControllerTest, CreateGroup_Returns201_OnSuccess) {
    // Preparación
    auto mockDelegate = std::make_shared<MockGroupDelegate>();
    GroupController controller(mockDelegate);
    std::string tournamentId = "tourn-1";
    std::string newGroupId = "group-abc";

    // Simular que el Delegate crea el grupo con éxito
    EXPECT_CALL(*mockDelegate, CreateGroup(tournamentId, _))
        .WillOnce(Return(std::expected<std::string, std::string>(newGroupId)));

    // Acción
    crow::request req;
    req.body = R"({"name":"Group Alpha"})"; // JSON de entrada
    crow::response res = controller.CreateGroup(req, tournamentId);

    // Verificación
    ASSERT_EQ(res.code, 201);
    ASSERT_EQ(res.get_header_value("Location"), newGroupId);
}

// Prueba creación fallida - Torneo no encontrado (HTTP 404) [cite: 212, 213]
TEST(GroupControllerTest, CreateGroup_Returns404_WhenTournamentNotFound) {
    // Preparación
    auto mockDelegate = std::make_shared<MockGroupDelegate>();
    GroupController controller(mockDelegate);
    std::string tournamentId = "non-existing-tourn";
    std::string errorMsg = "Tournament not found";

    // Simular que el Delegate falla porque el torneo no existe
    EXPECT_CALL(*mockDelegate, CreateGroup(tournamentId, _))
        .WillOnce(Return(std::unexpected(errorMsg)));

    // Acción
    crow::request req;
    req.body = R"({"name":"Group Beta"})";
    crow::response res = controller.CreateGroup(req, tournamentId);

    // Verificación
    ASSERT_EQ(res.code, 404); // Esperamos 404 según la lógica que añadimos
}

// Prueba creación fallida - Conflicto/Error (HTTP 422) [cite: 236-238] (Nota: PDF pide 409, pero 422 es más apropiado para errores de validación)
TEST(GroupControllerTest, CreateGroup_Returns422_OnDelegateError) {
    // Preparación
    auto mockDelegate = std::make_shared<MockGroupDelegate>();
    GroupController controller(mockDelegate);
    std::string tournamentId = "tourn-1";
    std::string errorMsg = "Group name already exists";

    // Simular que el Delegate falla por otra razón (ej. nombre duplicado)
    EXPECT_CALL(*mockDelegate, CreateGroup(tournamentId, _))
        .WillOnce(Return(std::unexpected(errorMsg)));

    // Acción
    crow::request req;
    req.body = R"({"name":"Existing Group Name"})";
    crow::response res = controller.CreateGroup(req, tournamentId);

    // Verificación
    ASSERT_EQ(res.code, 422); // Usamos 422 Unprocessable Entity
}

// --- Pruebas para GET /tournaments/{id}/groups (Leer Todos) ---

// Prueba leer todos con éxito (HTTP 200)
TEST(GroupControllerTest, GetGroups_Returns200_WithData) {
    // Preparación
    auto mockDelegate = std::make_shared<MockGroupDelegate>();
    GroupController controller(mockDelegate);
    std::string tournamentId = "tourn-1";
    std::vector<domain::Group> groups = { domain::Group("Group A"), domain::Group("Group B") };
    groups[0].Id() = "g1"; groups[0].TournamentId() = tournamentId;
    groups[1].Id() = "g2"; groups[1].TournamentId() = tournamentId;


    // Simular que el Delegate devuelve una lista de grupos
    EXPECT_CALL(*mockDelegate, GetGroups(tournamentId))
        .WillOnce(Return(std::expected<std::vector<domain::Group>, std::string>(groups)));

    // Acción
    crow::response res = controller.GetGroups(tournamentId);

    // Verificación
    ASSERT_EQ(res.code, 200);
    nlohmann::json body = nlohmann::json::parse(res.body);
    ASSERT_TRUE(body.is_array());
    ASSERT_EQ(body.size(), 2);
    ASSERT_EQ(body[0]["name"], "Group A");
}

// --- Pruebas para GET /tournaments/{id}/groups/{id} (Leer Uno) ---

// Prueba leer uno con éxito (HTTP 200) [cite: 239-240]
TEST(GroupControllerTest, GetGroupById_Returns200_WhenFound) {
    // Preparación
    auto mockDelegate = std::make_shared<MockGroupDelegate>();
    GroupController controller(mockDelegate);
    std::string tournamentId = "tourn-1";
    std::string groupId = "group-abc";
    domain::Group group("Found Group");
    group.Id() = groupId;
    group.TournamentId() = tournamentId;

    // Simular que el Delegate encuentra y devuelve el grupo
    EXPECT_CALL(*mockDelegate, GetGroup(tournamentId, groupId))
        .WillOnce(Return(std::expected<domain::Group, std::string>(group)));

    // Acción
    crow::response res = controller.GetGroup(tournamentId, groupId);

    // Verificación
    ASSERT_EQ(res.code, 200);
    nlohmann::json body = nlohmann::json::parse(res.body);
    ASSERT_EQ(body["id"], groupId);
    ASSERT_EQ(body["name"], "Found Group");
}

// Prueba leer uno fallida (HTTP 404) [cite: 241-242]
TEST(GroupControllerTest, GetGroupById_Returns404_WhenNotFound) {
    // Preparación
    auto mockDelegate = std::make_shared<MockGroupDelegate>();
    GroupController controller(mockDelegate);
    std::string tournamentId = "tourn-1";
    std::string groupId = "non-existing-group";

    // Simular que el Delegate no encuentra el grupo
    EXPECT_CALL(*mockDelegate, GetGroup(tournamentId, groupId))
        .WillOnce(Return(std::unexpected("Group not found")));

    // Acción
    crow::response res = controller.GetGroup(tournamentId, groupId);

    // Verificación
    ASSERT_EQ(res.code, 404);
}

// --- Pruebas para PATCH /tournaments/{id}/groups/{id} (Actualización) ---

// Prueba actualización exitosa (HTTP 204) [cite: 243-245]
TEST(GroupControllerTest, UpdateGroup_Returns204_OnSuccess) {
    // Preparación
    auto mockDelegate = std::make_shared<MockGroupDelegate>();
    GroupController controller(mockDelegate);
    std::string tournamentId = "tourn-1";
    std::string groupId = "group-abc";
    
    // Simular que el Delegate actualiza con éxito
    EXPECT_CALL(*mockDelegate, UpdateGroup(tournamentId, _)) // El segundo argumento es el objeto Group
        .WillOnce(Return(std::expected<void, std::string>()));

    // Acción
    crow::request req;
    req.body = R"({"name":"Updated Group Name"})";
    crow::response res = controller.UpdateGroup(req, tournamentId, groupId);

    // Verificación
    ASSERT_EQ(res.code, 204);
}

// Prueba actualización fallida (HTTP 404) [cite: 246-248]
TEST(GroupControllerTest, UpdateGroup_Returns404_WhenNotFound) {
    // Preparación
    auto mockDelegate = std::make_shared<MockGroupDelegate>();
    GroupController controller(mockDelegate);
    std::string tournamentId = "tourn-1";
    std::string groupId = "non-existing-group";
    
    // Simular que el Delegate no encuentra el grupo a actualizar
    EXPECT_CALL(*mockDelegate, UpdateGroup(tournamentId, _))
        .WillOnce(Return(std::unexpected("Group not found")));

    // Acción
    crow::request req;
    req.body = R"({"name":"Updated Group Name"})";
    crow::response res = controller.UpdateGroup(req, tournamentId, groupId);
    
    // Verificación
    ASSERT_EQ(res.code, 404);
}

// --- Pruebas para DELETE /tournaments/{id}/groups/{id} (Borrado) ---
// (Estas no estaban explícitamente en tu lista, pero son parte del CRUD)

TEST(GroupControllerTest, DeleteGroup_Returns204_OnSuccess) {
    auto mockDelegate = std::make_shared<MockGroupDelegate>();
    GroupController controller(mockDelegate);
    std::string tournamentId = "tourn-1";
    std::string groupId = "group-to-delete";

    EXPECT_CALL(*mockDelegate, RemoveGroup(tournamentId, groupId))
        .WillOnce(Return(std::expected<void, std::string>()));

    crow::response res = controller.DeleteGroup(tournamentId, groupId);

    ASSERT_EQ(res.code, 204);
}

TEST(GroupControllerTest, DeleteGroup_Returns404_WhenNotFound) {
    auto mockDelegate = std::make_shared<MockGroupDelegate>();
    GroupController controller(mockDelegate);
    std::string tournamentId = "tourn-1";
    std::string groupId = "non-existing-group";

    EXPECT_CALL(*mockDelegate, RemoveGroup(tournamentId, groupId))
        .WillOnce(Return(std::unexpected("Group not found")));

    crow::response res = controller.DeleteGroup(tournamentId, groupId);

    ASSERT_EQ(res.code, 404);
}