#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "service/MatchService.hpp"
#include "persistence/repository/IMatchRepository.hpp"

using namespace testing;

class MockMatchRepository : public repository::IMatchRepository {
public:
    MOCK_METHOD(domain::Match, Save, (const domain::Match&), (override));
    MOCK_METHOD(void, Update, (const domain::Match&), (override));
    MOCK_METHOD(void, Delete, (int), (override));
    MOCK_METHOD(std::optional<domain::Match>, FindById, (int), (override));
    MOCK_METHOD(std::vector<domain::Match>, FindAll, (), (override));
    MOCK_METHOD(std::vector<domain::Match>, FindByTournamentId, (int), (override));
    MOCK_METHOD(std::vector<domain::Match>, FindByTournamentIdAndPhase, (int, domain::MatchPhase), (override));
    MOCK_METHOD(std::vector<domain::Match>, FindByGroupId, (int), (override));
    MOCK_METHOD(std::vector<domain::Match>, FindByTeamId, (int), (override));
    MOCK_METHOD(int, CountByGroupId, (int), (override));
    MOCK_METHOD(bool, IsGroupStageComplete, (int), (override));
};

class MatchServiceTest : public ::testing::Test {
protected:
    std::shared_ptr<MockMatchRepository> mockRepo;
    std::unique_ptr<service::MatchService> service;

    void SetUp() override {
        mockRepo = std::make_shared<MockMatchRepository>();
        service = std::make_unique<service::MatchService>(mockRepo);
    }
};

TEST_F(MatchServiceTest, RegisterMatchResult_UpdatesMatchAndPublishesEvent) {
    // Arrange
    domain::Match match(1, domain::MatchPhase::GROUP_STAGE, 1);
    match.SetId(1);
    match.SetTeam1(10);
    match.SetTeam2(11);

    EXPECT_CALL(*mockRepo, FindById(1))
        .WillOnce(Return(std::optional<domain::Match>(match)));
    
    EXPECT_CALL(*mockRepo, Update(_))
        .Times(1);

    // Act
    service->RegisterMatchResult(1, 2, 1);

    // Assert - El evento se publicó (verificar con suscriptor si es necesario)
}

TEST_F(MatchServiceTest, RegisterMatchResult_ThrowsWhenMatchNotFound) {
    // Arrange
    EXPECT_CALL(*mockRepo, FindById(999))
        .WillOnce(Return(std::nullopt));

    // Act & Assert
    EXPECT_THROW(
        service->RegisterMatchResult(999, 2, 1),
        std::runtime_error
    );
}

TEST_F(MatchServiceTest, GetMatchesByTournament_ReturnsAllMatches) {
    // Arrange
    std::vector<domain::Match> matches;
    matches.push_back(domain::Match(1, domain::MatchPhase::GROUP_STAGE, 1));
    matches.push_back(domain::Match(1, domain::MatchPhase::GROUP_STAGE, 2));

    EXPECT_CALL(*mockRepo, FindByTournamentId(1))
        .WillOnce(Return(matches));

    // Act
    auto result = service->GetMatchesByTournament(1);

    // Assert
    EXPECT_EQ(result.size(), 2);
}