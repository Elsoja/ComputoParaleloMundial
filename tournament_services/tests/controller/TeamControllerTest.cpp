#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "controller/TeamController.hpp"
#include "delegate/ITeamDelegate.hpp"
#include "domain/Team.hpp"
#include "crow.h"
#include <expected>
#include <vector>
#include <memory>

using ::testing::Return;
using ::testing::_;

class MockTeamDelegate : public ITeamDelegate {
public:
    MOCK_METHOD((std::expected<std::string, SaveError>), SaveTeam, (const domain::Team& team), (override));
    MOCK_METHOD(std::shared_ptr<domain::Team>, GetTeam, (std::string_view id), (override));
    MOCK_METHOD(std::vector<std::shared_ptr<domain::Team>>, GetAllTeams, (), (override));
    MOCK_METHOD((std::expected<void, SaveError>), UpdateTeam, (std::string_view id, const domain::Team& team), (override));
    MOCK_METHOD((std::expected<void, SaveError>), DeleteTeam, (std::string_view id), (override));
};

// --- Pruebas para POST /teams (Creación) ---

// Prueba creación exitosa (HTTP 201)
TEST(TeamControllerTest, SaveTeam_Returns201_OnSuccess) {
    auto mockDelegate = std::make_shared<MockTeamDelegate>();
    TeamController controller(mockDelegate);
    std::string newId = "team-id-457";

    EXPECT_CALL(*mockDelegate, SaveTeam(_))
        .WillOnce(Return(std::expected<std::string, ITeamDelegate::SaveError>(newId)));

    crow::request req;
    req.body = "{\"name\":\"New Team\"}";
    crow::response res = controller.SaveTeam(req);

    ASSERT_EQ(res.code, 201);
    ASSERT_EQ(res.get_header_value("Location"), newId);
}

// Prueba creación fallida (HTTP 409)
TEST(TeamControllerTest, SaveTeam_Returns409_OnConflict) {
    auto mockDelegate = std::make_shared<MockTeamDelegate>();
    TeamController controller(mockDelegate);

    EXPECT_CALL(*mockDelegate, SaveTeam(_))
        .WillOnce(Return(std::unexpected(ITeamDelegate::SaveError::Conflict)));

    crow::request req;
    req.body = "{\"name\":\"Existing Team\"}";
    crow::response res = controller.SaveTeam(req);

    ASSERT_EQ(res.code, 409);
}

// --- Pruebas para GET /teams/{id} (Búsqueda por ID) ---

// Prueba búsqueda por ID exitosa (HTTP 200)
TEST(TeamControllerTest, GetTeamById_Returns200_WhenFound) {
    auto mockDelegate = std::make_shared<MockTeamDelegate>();
    TeamController controller(mockDelegate);
    std::string teamId = "existing-id-123";
    auto team = std::make_shared<domain::Team>(domain::Team{teamId, "Found Team"});

    EXPECT_CALL(*mockDelegate, GetTeam(teamId))
        .WillOnce(Return(team));

    crow::response res = controller.getTeam(teamId);

    ASSERT_EQ(res.code, 200);
    nlohmann::json body = nlohmann::json::parse(res.body);
    ASSERT_EQ(body["id"], teamId);
    ASSERT_EQ(body["name"], "Found Team");
}

// Prueba búsqueda por ID fallida (HTTP 404)
TEST(TeamControllerTest, GetTeamById_Returns404_WhenNotFound) {
    auto mockDelegate = std::make_shared<MockTeamDelegate>();
    TeamController controller(mockDelegate);
    std::string teamId = "non-existing-id-404";

    EXPECT_CALL(*mockDelegate, GetTeam(teamId))
        .WillOnce(Return(nullptr));

    crow::response res = controller.getTeam(teamId);

    ASSERT_EQ(res.code, 404);
}

// --- Pruebas para GET /teams (Búsqueda de todos) ---

// Prueba búsqueda de todos con resultados (HTTP 200)
TEST(TeamControllerTest, GetAllTeams_Returns200_WithListOfTeams) {
    auto mockDelegate = std::make_shared<MockTeamDelegate>();
    TeamController controller(mockDelegate);
    std::vector<std::shared_ptr<domain::Team>> teams = {
        std::make_shared<domain::Team>(domain::Team{"id1", "Team One"}),
        std::make_shared<domain::Team>(domain::Team{"id2", "Team Two"})
    };

    EXPECT_CALL(*mockDelegate, GetAllTeams())
        .WillOnce(Return(teams));

    crow::response res = controller.getAllTeams();
    
    ASSERT_EQ(res.code, 200);
    nlohmann::json body = nlohmann::json::parse(res.body);
    ASSERT_TRUE(body.is_array());
    ASSERT_EQ(body.size(), 2);
    ASSERT_EQ(body[0]["name"], "Team One");
}

// Prueba búsqueda de todos con lista vacía (HTTP 200)
TEST(TeamControllerTest, GetAllTeams_Returns200_WithEmptyList) {
    auto mockDelegate = std::make_shared<MockTeamDelegate>();
    TeamController controller(mockDelegate);
    std::vector<std::shared_ptr<domain::Team>> emptyList;

    EXPECT_CALL(*mockDelegate, GetAllTeams())
        .WillOnce(Return(emptyList));

    crow::response res = controller.getAllTeams();

    ASSERT_EQ(res.code, 200);
    nlohmann::json body = nlohmann::json::parse(res.body);
    ASSERT_TRUE(body.is_array());
    ASSERT_EQ(body.size(), 0);
}

// --- Pruebas para PATCH /teams/{id} (Actualización) ---

// Prueba actualización exitosa (HTTP 204)
TEST(TeamControllerTest, UpdateTeam_Returns204_OnSuccess) {
    auto mockDelegate = std::make_shared<MockTeamDelegate>();
    TeamController controller(mockDelegate);
    std::string teamId = "existing-id-123";
    
    EXPECT_CALL(*mockDelegate, UpdateTeam(teamId, _))
        .WillOnce(Return(std::expected<void, ITeamDelegate::SaveError>()));

    crow::request req;
    req.body = "{\"name\":\"Updated Name\"}";
    crow::response res = controller.UpdateTeam(req, teamId);

    ASSERT_EQ(res.code, 204);
}

// Prueba actualización fallida (HTTP 404)
TEST(TeamControllerTest, UpdateTeam_Returns404_WhenNotFound) {
    auto mockDelegate = std::make_shared<MockTeamDelegate>();
    TeamController controller(mockDelegate);
    std::string teamId = "non-existing-id-404";
    
    EXPECT_CALL(*mockDelegate, UpdateTeam(teamId, _))
        .WillOnce(Return(std::unexpected(ITeamDelegate::SaveError::NotFound)));

    crow::request req;
    req.body = "{\"name\":\"Updated Name\"}";
    crow::response res = controller.UpdateTeam(req, teamId);
    
    ASSERT_EQ(res.code, 404);
}

// --- Pruebas para DELETE /teams/{id} (Borrado) ---

// Prueba borrado exitoso (HTTP 204)
TEST(TeamControllerTest, DeleteTeam_Returns204_OnSuccess) {
    auto mockDelegate = std::make_shared<MockTeamDelegate>();
    TeamController controller(mockDelegate);
    std::string teamId = "existing-id-123";
    
    EXPECT_CALL(*mockDelegate, DeleteTeam(teamId))
        .WillOnce(Return(std::expected<void, ITeamDelegate::SaveError>()));

    crow::response res = controller.DeleteTeam(teamId);

    ASSERT_EQ(res.code, 204);
}

// Prueba borrado fallido (HTTP 404)
TEST(TeamControllerTest, DeleteTeam_Returns404_WhenNotFound) {
    auto mockDelegate = std::make_shared<MockTeamDelegate>();
    TeamController controller(mockDelegate);
    std::string teamId = "non-existing-id-404";
    
    EXPECT_CALL(*mockDelegate, DeleteTeam(teamId))
        .WillOnce(Return(std::unexpected(ITeamDelegate::SaveError::NotFound)));

    crow::response res = controller.DeleteTeam(teamId);
    
    ASSERT_EQ(res.code, 404);
}