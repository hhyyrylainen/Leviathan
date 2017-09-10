#include "Threading/ThreadingManager.h"
#include "TimeIncludes.h"

#include <future>
#include <chrono>

#include "catch.hpp"

using namespace Leviathan;

TEST_CASE("Task timing test", "[task][timing][threading][.slow]"){

    ThreadingManager manager;

    auto initresult = manager.Init();
    
    REQUIRE(initresult == true);
    
	int count = 0;

	std::promise<bool> Imdone;

    std::vector<int64_t> times;

    auto starttime = Time::GetTimeMs64();

	// Queue some tasks //
	manager.QueueTask(std::make_shared<RepeatCountedDelayedTask>(
            [&](){
                count++;
                
                times.push_back(Time::GetTimeMs64()-starttime);
                
                if(count == 5)
                    Imdone.set_value(true);
                
            }, std::chrono::milliseconds(50), 5));

	// Get time //
	auto timenow = Time::GetThreadSafeSteadyTimePoint();
	auto StartMicro = Time::GetTimeMs64();

	// Wait for them to finish //

	auto futureobj = Imdone.get_future();

    // This actually blocks until it is ran
	while(!futureobj.get()){
		// Allow stuff to run //
		manager.WaitForAllTasksToFinish();
	}

	// Check did it take long enough //
	auto timepassed = Time::GetThreadSafeSteadyTimePoint()-timenow;
	auto micropassed = Time::GetTimeMs64()-StartMicro;

	MillisecondDuration timeasmilli = std::chrono::duration_cast<MillisecondDuration>(
        timepassed);

	auto timeasmilliplain = timeasmilli.count();

    CHECK(timepassed > std::chrono::milliseconds(200));

    CHECK(timepassed < std::chrono::milliseconds(1500));

    manager.Release();
}

#include <iostream>

TEST_CASE("Tasks run", "[task][threading]"){

    ThreadingManager manager;

    auto initresult = manager.Init();
    
    REQUIRE(initresult == true);
    
    // Check that certain tasks actually run //
    std::atomic<int> repeatcountedruncount = {0};

    // Repeated tasks don't work correctly with wait for all tasks to finish
    manager.QueueTask(new RepeatCountedTask([&]() -> void
        {
            repeatcountedruncount++;
            // std::cout << "Ran increment: " << repeatcountedruncount << std::endl;
            
        }, 6));

    manager.WaitForAllTasksToFinish();

    // So we loop here to wait for all the tasks
    while(repeatcountedruncount != 6){

    }

    CHECK(repeatcountedruncount == 6);

    manager.Release();
}
