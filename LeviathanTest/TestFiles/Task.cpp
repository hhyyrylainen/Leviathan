#include "Threading/ThreadingManager.h"
#include "Common/Misc.h"

#include "boost/bind.hpp"

#include "catch.hpp"

using namespace Leviathan;

TEST_CASE("Task timing test", "[task, timing, threading]"){

    ThreadingManager manager;

    auto initresult = manager->Init();
    
    REQUIRE(initresult == true);
    
	int count = 0;

	boost::promise<bool> Imdone;

	vector<__int64> times;

	// Queue some tasks //
	manager.QueueTask(shared_ptr<QueuedTask>(new RepeatCountedDelayedTask(
                boost::bind<void>([](int &count, boost::promise<bool> &done, __int64 starttime, vector<__int64>* times)
                    {
                        count++;

                        times->push_back(Misc::GetTimeMs64()-starttime);

                        if(count == 5)
                            done.set_value(true);

                    }, count, boost::ref(Imdone), Misc::GetTimeMs64(), &times), boost::chrono::milliseconds(50), 5)));

	// Get time //
	auto timenow = Misc::GetThreadSafeSteadyTimePoint();
	__int64 StartMicro = Misc::GetTimeMs64();

	// Wait for them to finish //

	auto futureobj = Imdone.get_future();
	
	while(!futureobj.has_value()){
		// Allow stuff to run //
		manager.WaitForAllTasksToFinish();
	}

	// Check did it take long enough //
	auto timepassed = Misc::GetThreadSafeSteadyTimePoint()-timenow;
	__int64 micropassed = Misc::GetTimeMs64()-StartMicro;

	MillisecondDuration timeasmilli = boost::chrono::round<MillisecondDuration>(timepassed);

	__int64 timeasmilliplain = timeasmilli.count();

    CHECK(timepassed > boost::chrono::milliseconds(200));

    CHECK(timepassed < boost::chrono::milliseconds(410));

    manager.Release();
}

TEST_CASE("Tasks run", "[task, threading]"){

    ThreadingManager manager;

    auto initresult = manager->Init();
    
    REQUIRE(initresult == true);
    
    // Check that certain tasks actually run //
    int repeatcountedruncount = 0;
    
    manager.QueueTask(new RepeatCountedTask(boost::bind<void>([](int* count) -> void
        {
            (*count) = (*count)+1;
            
            
        }, &repeatcountedruncount), 6));


    manager.WaitForAllTasksToFinish();

    CHECK(repeatcountedruncount == 6);

    manager.Release();
}
