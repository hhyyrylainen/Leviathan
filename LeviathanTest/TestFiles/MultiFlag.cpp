#include "Utility/MultiFlag.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace std;

TEST_CASE("MultiFlag correctly sets values", "[flag]"){

    vector<shared_ptr<Flag>> flagvector;
	flagvector.push_back(make_shared<Flag>(2500));
	flagvector.push_back(make_shared<Flag>(2501));
	flagvector.push_back(make_shared<Flag>(2502));
	flagvector.push_back(make_shared<Flag>(2503));
	flagvector.push_back(make_shared<Flag>(2504));

	MultiFlag flags(flagvector);

	flags.SetFlag(Flag(2505));
	flags.UnsetFlag(Flag(2504));

    CHECK(flags.IsSet(2500) == true);

    CHECK(flags.IsSet(2501) == true);

    CHECK(flags.IsSet(2502) == true);

    CHECK(flags.IsSet(2503) == true);

    CHECK(flags.IsSet(2505) == true);

    CHECK(flags.IsSet(2504) == false);
}
