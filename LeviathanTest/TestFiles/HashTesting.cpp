#include "Utility/MD5Generator.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace std;

TEST_CASE("MD5 makes correct hexdigest", "[hash]"){
    
	const string testvalue = "string to make into md5 has!";

    const string result = MD5(testvalue).hexdigest();

    CHECK(result == "a59e6c8c49baf73cb6c15dbc18967812");
}
