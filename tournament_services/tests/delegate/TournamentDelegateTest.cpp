#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "delegate/TournamentDelegate.hpp"
#include "persistence/repository/IRepository.hpp"
#include "domain/Tournament.hpp"
#include "cms/QueueMessageProducer.hpp" // Necesario para el mock
#include "cms/ConnectionManager.hpp"    // Incluir ConnectionManager
#include <optional>
#include <vector>
#include <memory>
#include <string> // Incluir string para el mock de repositorio

// Usamos 'using' para simplificar las llamadas
using ::testing::Return;
using ::testing::_;

// --- Mocks Necesarios ---

// Mock del Repositorio de Torneos
class MockTournamentRepository : public IRepository<domain::Tournament, std::string> {
public:
    MOCK_METHOD(std::optional<std::string>, Create, (const domain::Tournament& entity), (override));
    MOCK_METHOD(std::shared_ptr<domain::Tournament>, ReadById, (std::string id), (override));
    MOCK_METHOD(std::vector<std::shared_ptr<domain::Tournament>>, ReadAll, (), (override));
    MOCK_METHOD(std::string, Update, (const domain::Tournament& entity), (override));
    MOCK_METHOD(void, Delete, (std::string id), (override));
};

// Mock del Productor de Mensajes (necesario para el constructor del Delegate)
class MockQueueMessageProducer : public QueueMessageProducer {
public:
    // Necesitamos definir el constructor base
    MockQueueMessageProducer(std::shared_ptr<ConnectionManager> manager) : QueueMessageProducer(manager) {}

    // CORRECCIÓN: Se quitó 'override' porque SendMessage no es virtual en la clase base QueueMessageProducer.
    //             La firma también debe coincidir exactamente (std::string vs std::string_view).
    MOCK_METHOD(void, SendMessage, (const std::string& message, const std::string& queueName));
};

// --- Pruebas para Creación ---

// Prueba de creación exitosa [cite: 154, 155]
TEST(TournamentDelegateTest, CreateTournament_Success) {
    // Preparación
    auto mockRepo = std::make_shared<MockTournamentRepository>();
    auto mockConnManager = std::make_shared<ConnectionManager>(); // No necesita ser mockeado si no se usa su lógica
    auto mockProducer = std::make_shared<MockQueueMessageProducer>(mockConnManager);
    TournamentDelegate delegate(mockRepo, mockProducer);

    auto newTournament = std::make_shared<domain::Tournament>("Summer Cup");
    std::string expectedId = "tourn-uuid-456";

    // Simular que el repositorio devuelve un ID de éxito
    // Usamos el objeto *newTournament para la comparación
    EXPECT_CALL(*mockRepo, Create(*newTournament))
        .WillOnce(Return(std::optional<std::string>(expectedId)));
    // Simular que el productor de mensajes es llamado con el ID correcto
    EXPECT_CALL(*mockProducer, SendMessage(expectedId, "tournament.created"));

    // Acción
    auto result = delegate.CreateTournament(newTournament);

    // Verificación
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), expectedId);
}

// Prueba de creación fallida (conflicto) [cite: 156-158]
TEST(TournamentDelegateTest, CreateTournament_Conflict) {
    // Preparación
    auto mockRepo = std::make_shared<MockTournamentRepository>();
    auto mockConnManager = std::make_shared<ConnectionManager>();
    auto mockProducer = std::make_shared<MockQueueMessageProducer>(mockConnManager);
    TournamentDelegate delegate(mockRepo, mockProducer);

    auto existingTournament = std::make_shared<domain::Tournament>("Winter Cup");

    // Simular que el repositorio falla (devuelve optional vacío)
    EXPECT_CALL(*mockRepo, Create(*existingTournament))
        .WillOnce(Return(std::nullopt));
    // El productor de mensajes NO debe ser llamado si la creación falla
    EXPECT_CALL(*mockProducer, SendMessage(_, _)).Times(0);

    // Acción
    auto result = delegate.CreateTournament(existingTournament);

    // Verificación
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), ITournamentDelegate::SaveError::Conflict);
}

// --- Pruebas para Búsqueda por ID ---

// Prueba de búsqueda por ID exitosa [cite: 159, 160]
TEST(TournamentDelegateTest, GetTournament_ReturnsTournament_WhenFound) {
    // Preparación
    auto mockRepo = std::make_shared<MockTournamentRepository>();
    auto mockConnManager = std::make_shared<ConnectionManager>();
    auto mockProducer = std::make_shared<MockQueueMessageProducer>(mockConnManager);
    TournamentDelegate delegate(mockRepo, mockProducer);

    std::string tournamentId = "t1";
    auto expectedTournament = std::make_shared<domain::Tournament>("Tournament One");
    expectedTournament->Id() = tournamentId;

    // Simular que el repositorio encuentra y devuelve el torneo
    EXPECT_CALL(*mockRepo, ReadById(tournamentId))
        .WillOnce(Return(expectedTournament));

    // Acción
    auto result = delegate.GetTournament(tournamentId);

    // Verificación
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->Id(), tournamentId); // Validar valores del objeto
    ASSERT_EQ(result->Name(), "Tournament One");
}

// Prueba de búsqueda por ID fallida (no encontrado) [cite: 165, 166]
TEST(TournamentDelegateTest, GetTournament_ReturnsNull_WhenNotFound) {
    // Preparación
    auto mockRepo = std::make_shared<MockTournamentRepository>();
    auto mockConnManager = std::make_shared<ConnectionManager>();
    auto mockProducer = std::make_shared<MockQueueMessageProducer>(mockConnManager);
    TournamentDelegate delegate(mockRepo, mockProducer);

    std::string nonExistingId = "non-existing-id";

    // Simular que el repositorio no encuentra nada
    EXPECT_CALL(*mockRepo, ReadById(nonExistingId))
        .WillOnce(Return(nullptr));

    // Acción
    auto result = delegate.GetTournament(nonExistingId);

    // Verificación
    ASSERT_EQ(result, nullptr); // Validar nullptr
}

// --- Pruebas para Búsqueda de Todos ---

// Prueba de búsqueda de todos con resultados [cite: 167]
TEST(TournamentDelegateTest, GetAllTournaments_ReturnsListOfTournaments) {
    // Preparación
    auto mockRepo = std::make_shared<MockTournamentRepository>();
    auto mockConnManager = std::make_shared<ConnectionManager>();
    auto mockProducer = std::make_shared<MockQueueMessageProducer>(mockConnManager);
    TournamentDelegate delegate(mockRepo, mockProducer);

    std::vector<std::shared_ptr<domain::Tournament>> tournaments = {
        std::make_shared<domain::Tournament>("Tournament One"),
        std::make_shared<domain::Tournament>("Tournament Two")
    };

    // Simular que el repositorio devuelve una lista con torneos
    EXPECT_CALL(*mockRepo, ReadAll())
        .WillOnce(Return(tournaments));

    // Acción
    auto result = delegate.GetAllTournaments();

    // Verificación
    ASSERT_EQ(result.size(), 2);
}

// Prueba de búsqueda de todos con lista vacía [cite: 168]
TEST(TournamentDelegateTest, GetAllTournaments_ReturnsEmptyList) {
    // Preparación
    auto mockRepo = std::make_shared<MockTournamentRepository>();
    auto mockConnManager = std::make_shared<ConnectionManager>();
    auto mockProducer = std::make_shared<MockQueueMessageProducer>(mockConnManager);
    TournamentDelegate delegate(mockRepo, mockProducer);

    // Simular que el repositorio devuelve una lista vacía
    EXPECT_CALL(*mockRepo, ReadAll())
        .WillOnce(Return(std::vector<std::shared_ptr<domain::Tournament>>()));

    // Acción
    auto result = delegate.GetAllTournaments();

    // Verificación
    ASSERT_TRUE(result.empty());
}

// --- Pruebas para Actualización ---

// Prueba de actualización exitosa [cite: 169, 170]
TEST(TournamentDelegateTest, UpdateTournament_ReturnsSuccess_WhenTournamentExists) {
    // Preparación
    auto mockRepo = std::make_shared<MockTournamentRepository>();
    auto mockConnManager = std::make_shared<ConnectionManager>();
    auto mockProducer = std::make_shared<MockQueueMessageProducer>(mockConnManager);
    TournamentDelegate delegate(mockRepo, mockProducer);

    std::string tournamentId = "existing-tourn-id";
    auto existingTournament = std::make_shared<domain::Tournament>("Original Name");
    existingTournament->Id() = tournamentId;
    domain::Tournament updatedTournament("Updated Name");
    updatedTournament.Id() = tournamentId;

    // Simular que primero se encuentra el torneo
    EXPECT_CALL(*mockRepo, ReadById(tournamentId))
        .WillOnce(Return(existingTournament));
    // Simular que la actualización en el repositorio es exitosa
    EXPECT_CALL(*mockRepo, Update(updatedTournament))
        .WillOnce(Return(tournamentId)); // Simula que Update devuelve el ID

    // Acción
    auto result = delegate.UpdateTournament(tournamentId, updatedTournament);

    // Verificación
    ASSERT_TRUE(result.has_value()); // Simular resultado exitoso
}

// Prueba de actualización fallida (no encontrado) [cite: 171, 172]
TEST(TournamentDelegateTest, UpdateTournament_ReturnsNotFound_WhenTournamentDoesNotExist) {
    // Preparación
    auto mockRepo = std::make_shared<MockTournamentRepository>();
    auto mockConnManager = std::make_shared<ConnectionManager>();
    auto mockProducer = std::make_shared<MockQueueMessageProducer>(mockConnManager);
    TournamentDelegate delegate(mockRepo, mockProducer);

    std::string tournamentId = "non-existing-tourn-id";
    domain::Tournament updatedTournament("Updated Name");
    updatedTournament.Id() = tournamentId;

    // Simular que el torneo a actualizar NO es encontrado
    EXPECT_CALL(*mockRepo, ReadById(tournamentId))
        .WillOnce(Return(nullptr));
    // El método Update del repositorio NO debe ser llamado
    EXPECT_CALL(*mockRepo, Update(_)).Times(0);

    // Acción
    auto result = delegate.UpdateTournament(tournamentId, updatedTournament);

    // Verificación
    ASSERT_FALSE(result.has_value()); // Regresar error
    ASSERT_EQ(result.error(), ITournamentDelegate::SaveError::NotFound); // Mensaje usando expected
}