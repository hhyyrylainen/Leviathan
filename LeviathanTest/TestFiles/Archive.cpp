#include "Common/SFMLPackets.h"
#include "Generated/ComponentStates.h"
#include "Entities/Components.h"

#include "catch.hpp"

using namespace Leviathan;


TEST_CASE("Nested sf::Packets work correctly", "[networking]")
{
    sf::Packet outer;
    sf::Packet inner;

    SECTION("empty")
    {
        CHECK(inner.getDataSize() == 0);
        outer << inner;

        CHECK(outer.getDataSize() == sizeof(uint32_t));

        sf::Packet result;
        outer >> result;

        REQUIRE(outer);
        CHECK(result.getDataSize() == 0);
    }

    SECTION("with int32 nested payload")
    {
        inner << int32_t(12);
        CHECK(inner.getDataSize() == sizeof(int32_t));
        outer << inner;

        CHECK(outer.getDataSize() == sizeof(uint32_t) + inner.getDataSize());

        SECTION("manual decode")
        {
            uint32_t length;
            outer >> length;

            REQUIRE(outer);
            CHECK(length == inner.getDataSize());
        }

        SECTION("string decode (this is an implementation detail...)")
        {
            std::string data;
            outer >> data;

            REQUIRE(outer);
            CHECK(data.size() == inner.getDataSize());
        }

        SECTION("SFML extract operator")
        {
            sf::Packet result;
            outer >> result;

            REQUIRE(outer);
            CHECK(result.getDataSize() == sizeof(int32_t));

            int32_t retrieved = -1;
            result >> retrieved;

            REQUIRE(result);
            CHECK(retrieved == 12);
        }
    }
}

TEST_CASE("Loading PositionState from directly created packet works", "[networking][entity]")
{
    sf::Packet data;

    const Float3 pos = {1, 2, 3};
    const Float4 orientation = {4, 5, 6, 7};

    PositionState state(1, pos, orientation);
    state.AddDataToPacket(data, nullptr);

    uint16_t type;
    data >> type;
    REQUIRE(data);

    CHECK(type == static_cast<uint16_t>(Position::TYPE));

    PositionState loaded(nullptr, data);
    CHECK(data);

    CHECK(loaded._Position == pos);
    CHECK(loaded._Orientation == orientation);
}

