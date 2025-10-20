#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "controller/TournamentController.hpp"
#include "delegate/ITournamentDelegate.hpp"
#include "domain/Tournament.hpp"
#include "crow.h"
#include <expected>
#include <vector>

// Usamos 'using' para simplificar las llamadas a los matchers de gmock
using ::testing::Return;
using ::testing::_; // El comodín para "cualquier argumento"

// Clase Mock para simular el TournamentDelegate
class MockTournamentDelegate : public ITournamentDelegate {
public:
    // Mocks para todos los métodos de la interfaz ITournamentDelegate
    MOCK_METHOD((std::expected<std::string, SaveError>), CreateTournament, (std::shared_ptr<domain::Tournament> tournament), (override));
    MOCK_METHOD(std::shared_ptr<domain::Tournament>, GetTournament, (std::string_view id), (override));
    MOCK_METHOD(std::vector<std::shared_ptr<domain::Tournament>>, GetAllTournaments, (), (override));
    MOCK_METHOD((std::expected<void, SaveError>), UpdateTournament, (std::string_view id, const domain::Tournament& tournament), (override));
    MOCK_METHOD((std::expected<void, SaveError>), DeleteTournament, (std::string_view id), (override));
};

// --- Pruebas para POST /tournaments (Creación) ---

TEST(TournamentControllerTest, CreateTournament_Returns201_OnSuccess) {
    // Preparación
    auto mockDelegate = std::make_shared<MockTournamentDelegate>();
    TournamentController controller(mockDelegate);
    std::string newId = "tournament-id-xyz";

    // Simular que el Delegate crea el torneo con éxito
    EXPECT_CALL(*mockDelegate, CreateTournament(_))
        .WillOnce(Return(std::expected<std::string, ITournamentDelegate::SaveError>(newId)));

    // Acción
    crow::request req;
    req.body = "{\"name\":\"New Tournament\"}"; // Cuerpo JSON para crear
    crow::response res = controller.CreateTournament(req);

    // Verificación [cite: 136-139]
    ASSERT_EQ(res.code, 201);
    ASSERT_EQ(res.get_header_value("Location"), newId);
}

TEST(TournamentControllerTest, CreateTournament_Returns409_OnConflict) {
    // Preparación
    auto mockDelegate = std::make_shared<MockTournamentDelegate>();
    TournamentController controller(mockDelegate);

    // Simular que el Delegate falla por conflicto
    EXPECT_CALL(*mockDelegate, CreateTournament(_))
        .WillOnce(Return(std::unexpected(ITournamentDelegate::SaveError::Conflict)));

    // Acción
    crow::request req;
    req.body = "{\"name\":\"Existing Tournament\"}";
    crow::response res = controller.CreateTournament(req);

    // Verificación [cite: 140-141]
    ASSERT_EQ(res.code, 409);
}

// --- Pruebas para GET /tournaments/{id} (Búsqueda por ID) ---

TEST(TournamentControllerTest, GetTournamentById_Returns200_WhenFound) {
    // Preparación
    auto mockDelegate = std::make_shared<MockTournamentDelegate>();
    TournamentController controller(mockDelegate);
    std::string tournamentId = "existing-tourn-id";
    // Crear un objeto torneo simulado
    auto tournament = std::make_shared<domain::Tournament>("Found Tournament");
    tournament->Id() = tournamentId; // Asignar ID para la verificación

    // Simular que el Delegate encuentra y devuelve el torneo
    EXPECT_CALL(*mockDelegate, GetTournament(tournamentId))
        .WillOnce(Return(tournament));

    // Acción
    crow::response res = controller.GetTournament(tournamentId);

    // Verificación [cite: 142-143]
    ASSERT_EQ(res.code, 200);
    nlohmann::json body = nlohmann::json::parse(res.body);
    ASSERT_EQ(body["id"], tournamentId);
    ASSERT_EQ(body["name"], "Found Tournament");
}

TEST(TournamentControllerTest, GetTournamentById_Returns404_WhenNotFound) {
    // Preparación
    auto mockDelegate = std::make_shared<MockTournamentDelegate>();
    TournamentController controller(mockDelegate);
    std::string tournamentId = "non-existing-tourn-id";

    // Simular que el Delegate NO encuentra el torneo (devuelve nullptr)
    EXPECT_CALL(*mockDelegate, GetTournament(tournamentId))
        .WillOnce(Return(nullptr));

    // Acción
    crow::response res = controller.GetTournament(tournamentId);

    // Verificación [cite: 144-145]
    ASSERT_EQ(res.code, 404);
}

// --- Pruebas para GET /tournaments (Búsqueda de todos) ---

TEST(TournamentControllerTest, GetAllTournaments_Returns200_WithListOfTournaments) {
    // Preparación
    auto mockDelegate = std::make_shared<MockTournamentDelegate>();
    TournamentController controller(mockDelegate);
    std::vector<std::shared_ptr<domain::Tournament>> tournaments = {
        std::make_shared<domain::Tournament>("Tournament One"),
        std::make_shared<domain::Tournament>("Tournament Two")
    };
    // Asignar IDs para verificar el JSON
    tournaments[0]->Id() = "id1";
    tournaments[1]->Id() = "id2";


    // Simular que el Delegate devuelve una lista con dos torneos
    EXPECT_CALL(*mockDelegate, GetAllTournaments())
        .WillOnce(Return(tournaments));

    // Acción
    crow::response res = controller.ReadAll();
    
    // Verificación [cite: 146]
    ASSERT_EQ(res.code, 200);
    nlohmann::json body = nlohmann::json::parse(res.body);
    ASSERT_TRUE(body.is_array());
    ASSERT_EQ(body.size(), 2);
    ASSERT_EQ(body[0]["name"], "Tournament One");
}

TEST(TournamentControllerTest, GetAllTournaments_Returns200_WithEmptyList) {
    // Preparación
    auto mockDelegate = std::make_shared<MockTournamentDelegate>();
    TournamentController controller(mockDelegate);
    std::vector<std::shared_ptr<domain::Tournament>> emptyList;

    // Simular que el Delegate devuelve una lista vacía
    EXPECT_CALL(*mockDelegate, GetAllTournaments())
        .WillOnce(Return(emptyList));

    // Acción
    crow::response res = controller.ReadAll();

    // Verificación [cite: 147]
    ASSERT_EQ(res.code, 200);
    nlohmann::json body = nlohmann::json::parse(res.body);
    ASSERT_TRUE(body.is_array());
    ASSERT_EQ(body.size(), 0);
}

// --- Pruebas para PATCH /tournaments/{id} (Actualización) ---

TEST(TournamentControllerTest, UpdateTournament_Returns204_OnSuccess) {
    // Preparación
    auto mockDelegate = std::make_shared<MockTournamentDelegate>();
    TournamentController controller(mockDelegate);
    std::string tournamentId = "existing-tourn-id";
    
    // Simular que el Delegate actualiza con éxito
    EXPECT_CALL(*mockDelegate, UpdateTournament(tournamentId, _))
        .WillOnce(Return(std::expected<void, ITournamentDelegate::SaveError>()));

    // Acción
    crow::request req;
    req.body = "{\"name\":\"Updated Tournament Name\"}";
    crow::response res = controller.UpdateTournament(req, tournamentId);

    // Verificación [cite: 148-149]
    ASSERT_EQ(res.code, 204);
}

TEST(TournamentControllerTest, UpdateTournament_Returns404_WhenNotFound) {
    // Preparación
    auto mockDelegate = std::make_shared<MockTournamentDelegate>();
    TournamentController controller(mockDelegate);
    std::string tournamentId = "non-existing-tourn-id";
    
    // Simular que el Delegate no encuentra el torneo a actualizar
    EXPECT_CALL(*mockDelegate, UpdateTournament(tournamentId, _))
        .WillOnce(Return(std::unexpected(ITournamentDelegate::SaveError::NotFound)));

    // Acción
    crow::request req;
    req.body = "{\"name\":\"Updated Tournament Name\"}";
    crow::response res = controller.UpdateTournament(req, tournamentId);
    
    // Verificación [cite: 150-151]
    ASSERT_EQ(res.code, 404);
}