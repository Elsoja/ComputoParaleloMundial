#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "delegate/TournamentDelegate.hpp"
#include "persistence/repository/IRepository.hpp"
#include "domain/Tournament.hpp"
#include "cms/IQueueMessageProducer.hpp" // ✅ CAMBIO: Se incluye la INTERFAZ
#include <optional>
#include <vector>
#include <memory>
#include <string>

// Usamos 'using' para simplificar las llamadas
using ::testing::Return;
using ::testing::_;
using ::testing::Eq; // Para comparar objetos

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

// ✅ CAMBIO: El MockProducer ahora hereda de la INTERFAZ IQueueMessageProducer
class MockQueueMessageProducer : public IQueueMessageProducer {
public:
    // Ya no necesita un constructor con ConnectionManager
    MOCK_METHOD(void, SendMessage, (const std::string_view& message, const std::string_view& queue), (override));
};

// --- Pruebas para Creación ---

// Prueba de creación exitosa
TEST(TournamentDelegateTest, CreateTournament_Success) {
    // Preparación
    auto mockRepo = std::make_shared<MockTournamentRepository>();
    // ✅ CAMBIO: Se crea el mock sin argumentos
    auto mockProducer = std::make_shared<MockQueueMessageProducer>(); 
    TournamentDelegate delegate(mockRepo, mockProducer);
    
    auto newTournament = std::make_shared<domain::Tournament>("Summer Cup");
    std::string expectedId = "tourn-uuid-456";

    // Simular que el repositorio devuelve un ID de éxito
    EXPECT_CALL(*mockRepo, Create(Eq(*newTournament)))
        .WillOnce(Return(std::optional<std::string>(expectedId)));
    // Simular que el productor de mensajes es llamado con el ID correcto
    EXPECT_CALL(*mockProducer, SendMessage(std::string_view(expectedId), std::string_view("tournament.created")));

    // Acción
    auto result = delegate.CreateTournament(newTournament);

    // Verificación
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), expectedId);
}

// Prueba de creación fallida (conflicto)
TEST(TournamentDelegateTest, CreateTournament_Conflict) {
    // Preparación
    auto mockRepo = std::make_shared<MockTournamentRepository>();
    auto mockProducer = std::make_shared<MockQueueMessageProducer>();
    TournamentDelegate delegate(mockRepo, mockProducer);

    auto existingTournament = std::make_shared<domain::Tournament>("Winter Cup");

    // Simular que el repositorio falla (devuelve optional vacío)
    EXPECT_CALL(*mockRepo, Create(Eq(*existingTournament)))
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

// Prueba de búsqueda por ID exitosa
TEST(TournamentDelegateTest, GetTournament_ReturnsTournament_WhenFound) {
    // Preparación
    auto mockRepo = std::make_shared<MockTournamentRepository>();
    auto mockProducer = std::make_shared<MockQueueMessageProducer>();
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
    ASSERT_EQ(result->Id(), tournamentId);
    ASSERT_EQ(result->Name(), "Tournament One");
}

// Prueba de búsqueda por ID fallida (no encontrado)
TEST(TournamentDelegateTest, GetTournament_ReturnsNull_WhenNotFound) {
    // Preparación
    auto mockRepo = std::make_shared<MockTournamentRepository>();
    auto mockProducer = std::make_shared<MockQueueMessageProducer>();
    TournamentDelegate delegate(mockRepo, mockProducer);
    
    // Simular que el repositorio no encuentra nada
    EXPECT_CALL(*mockRepo, ReadById(_))
        .WillOnce(Return(nullptr));

    // Acción
    auto result = delegate.GetTournament("non-existing-id");

    // Verificación
    ASSERT_EQ(result, nullptr);
}

// --- Pruebas para Búsqueda de Todos ---

// Prueba de búsqueda de todos con resultados
TEST(TournamentDelegateTest, GetAllTournaments_ReturnsListOfTournaments) {
    // Preparación
    auto mockRepo = std::make_shared<MockTournamentRepository>();
    auto mockProducer = std::make_shared<MockQueueMessageProducer>();
    TournamentDelegate delegate(mockRepo, mockProducer);

    std::vector<std::shared_ptr<domain::Tournament>> tournaments = {
        std::make_shared<domain::Tournament>("Tournament One")
    };

    // Simular que el repositorio devuelve una lista con un torneo
    EXPECT_CALL(*mockRepo, ReadAll())
        .WillOnce(Return(tournaments));
    
    // Acción
    auto result = delegate.GetAllTournaments();

    // Verificación
    ASSERT_EQ(result.size(), 1);
}

// Prueba de búsqueda de todos con lista vacía
TEST(TournamentDelegateTest, GetAllTournaments_ReturnsEmptyList) {
    // Preparación
    auto mockRepo = std::make_shared<MockTournamentRepository>();
    auto mockProducer = std::make_shared<MockQueueMessageProducer>();
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

// Prueba de actualización exitosa
TEST(TournamentDelegateTest, UpdateTournament_ReturnsSuccess_WhenTournamentExists) {
    // Preparación
    auto mockRepo = std::make_shared<MockTournamentRepository>();
    auto mockProducer = std::make_shared<MockQueueMessageProducer>();
    TournamentDelegate delegate(mockRepo, mockProducer);

    std::string tournamentId = "existing-tourn-id";
    auto existingTournament = std::make_shared<domain::Tournament>("Original Name");
    existingTournament->Id() = tournamentId;
    domain::Tournament updatedTournament("Updated Name");
    updatedTournament.Id() = tournamentId;

    // Simular que primero se encuentra el torneo
    EXPECT_CALL(*mockRepo, ReadById(tournamentId))
        .WillOnce(Return(existingTournament));
    // Simular que la actualización es exitosa
    EXPECT_CALL(*mockRepo, Update(Eq(updatedTournament)))
        .WillOnce(Return(tournamentId));

    // Acción
    auto result = delegate.UpdateTournament(tournamentId, updatedTournament);

    // Verificación
    ASSERT_TRUE(result.has_value());
}

// Prueba de actualización fallida (no encontrado)
TEST(TournamentDelegateTest, UpdateTournament_ReturnsNotFound_WhenTournamentDoesNotExist) {
    // Preparación
    auto mockRepo = std::make_shared<MockTournamentRepository>();
    auto mockProducer = std::make_shared<MockQueueMessageProducer>();
    TournamentDelegate delegate(mockRepo, mockProducer);

    std::string tournamentId = "non-existing-tourn-id";
    domain::Tournament updatedTournament("Updated Name");
    updatedTournament.Id() = tournamentId;

    // Simular que el torneo a actualizar no es encontrado
    EXPECT_CALL(*mockRepo, ReadById(tournamentId))
        .WillOnce(Return(nullptr));

    // Acción
    auto result = delegate.UpdateTournament(tournamentId, updatedTournament);

    // Verificación
    ASSERT_FALSE(result.has_value());
    ASSERT_EQ(result.error(), ITournamentDelegate::SaveError::NotFound);
}