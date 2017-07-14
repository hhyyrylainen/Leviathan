#include "Handlers/IDFactory.h"

#include "catch.hpp"

#include <memory>

using namespace Leviathan;

TEST_CASE("IDFactory no repeats", "[ID]"){
#define RUN_COUNT 60

    IDFactory factory;
    
    std::vector<int> ids;

    ids.resize(RUN_COUNT);

    for(int i = 0; i < RUN_COUNT; i++){

        ids[i] = IDFactory::GetID();
	}

    for(size_t i = 0; i < ids.size(); ++i){
        for(size_t a = 0; a < ids.size(); ++a){

            if(i == a)
                continue;

            // Only do check when this fails to keep the
            // reported assertion count down
            if(ids[i] == ids[a])
                CHECK(ids[i] != ids[a]);
        }
    }
}
