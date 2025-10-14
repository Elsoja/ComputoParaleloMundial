#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "delegate/TeamDelegate.hpp"
#include "persistence/repository/IRepository.hpp"
#include "domain/Team.hpp"
#include <optional>

class MockTeamRepository : public IRepository<domain::Team, std::string> {
public:
    // CAMBIO: Se corrigió la firma del mock para que coincida con la interfaz
    MOCK_METHOD(std::optional<std::string>, Create, (const domain::Team& entity), (override));
    MOCK_METHOD(std::shared_ptr<domain::Team>, ReadById, (std::string id), (override));
    MOCK_METHOD(std::vector<std::shared_ptr<domain::Team>>, ReadAll, (), (override));
    MOCK_METHOD(std::string, Update, (const domain::Team& entity), (override));
    MOCK_METHOD(void, Delete, (std::string id), (override));
};

TEST(TeamDelegateTest, SaveTeam_Success) {
    auto mockRepo = std::make_shared<MockTeamRepository>();
    TeamDelegate delegate(mockRepo);
    domain::Team newTeam{"", "Team A"};
    std::string expectedId = "new-uuid-122";

    EXPECT_CALL(*mockRepo, Create(testing::_))
        .WillOnce(testing::Return(std::optional<std::string>(expectedId)));

    auto result = delegate.SaveTeam(newTeam);

    // CAMBIO: Se ajustaron las aserciones para std::optional
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), expectedId);
}

TEST(TeamDelegateTest, SaveTeam_Conflict) {
    auto mockRepo = std::make_shared<MockTeamRepository>();
    TeamDelegate delegate(mockRepo);
    domain::Team existingTeam{"", "Team B"};

    EXPECT_CALL(*mockRepo, Create(testing::_))
        .WillOnce(testing::Return(std::nullopt));

    auto result = delegate.SaveTeam(existingTeam);

    // CAMBIO: Se ajustó la aserción para un optional vacío
    ASSERT_FALSE(result.has_value());
}