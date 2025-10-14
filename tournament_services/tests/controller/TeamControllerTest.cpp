#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "controller/TeamController.hpp"
#include "delegate/ITeamDelegate.hpp"
#include "crow.h"
#include <optional> // CAMBIO: Incluir optional

class MockTeamDelegate : public ITeamDelegate {
public:
    // CAMBIO: Se corrigió la firma del mock. El paréntesis extra ya no es necesario
    // porque el tipo ya no tiene comas, pero es buena práctica mantenerlo.
    MOCK_METHOD(std::optional<std::string>, SaveTeam, (const domain::Team& team), (override));
    MOCK_METHOD(std::shared_ptr<domain::Team>, GetTeam, (std::string_view id), (override));
    MOCK_METHOD(std::vector<std::shared_ptr<domain::Team>>, GetAllTeams, (), (override));
};

TEST(TeamControllerTest, SaveTeam_Returns201_OnSuccess) {
    auto mockDelegate = std::make_shared<MockTeamDelegate>();
    TeamController controller(mockDelegate);
    std::string newId = "team-id-457";

    // CAMBIO: El mock ahora devuelve un std::optional
    EXPECT_CALL(*mockDelegate, SaveTeam(testing::_))
        .WillOnce(testing::Return(std::optional<std::string>(newId)));

    crow::request req;
    req.method = crow::HTTPMethod::Post;
    req.body = "{\"name\":\"New Team\"}";

    crow::response res = controller.SaveTeam(req);

    EXPECT_EQ(res.code, 201);
    EXPECT_EQ(res.get_header_value("Location"), newId);
}

TEST(TeamControllerTest, SaveTeam_Returns409_OnConflict) {
    auto mockDelegate = std::make_shared<MockTeamDelegate>();
    TeamController controller(mockDelegate);

    // CAMBIO: El mock ahora devuelve un optional vacío (std::nullopt)
    EXPECT_CALL(*mockDelegate, SaveTeam(testing::_))
        .WillOnce(testing::Return(std::nullopt));

    crow::request req;
    req.method = crow::HTTPMethod::Post;
    req.body = "{\"name\":\"Existing Team\"}";

    crow::response res = controller.SaveTeam(req);

    EXPECT_EQ(res.code, 409);
}