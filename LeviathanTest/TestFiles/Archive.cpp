#include "Common/SFMLPackets.h"

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
