#include <thread>
#include <mutex>
#include <functional>

#include "catch.hpp"

using namespace std;

int TestChanger(int iarg, double idouble, void* output){

    *reinterpret_cast<int*>(output) = iarg + static_cast<int>(idouble);
    
    return 14;
}

void ThreadRunner(mutex &QueueMutex, vector<shared_ptr<function<int ()>>> &Functions){

    std::lock_guard<std::mutex> locked(QueueMutex);

    while(!Functions.empty()){

        (*Functions[Functions.size()-1])();
        Functions.pop_back();
    }
}

TEST_CASE("std::function passing between threads", "[std]"){

    mutex QueueMutex;
    vector<shared_ptr<function<int ()>>> Functions;

    int target = -12;
    
    auto torun = make_shared<function<int ()>>(bind(&TestChanger, 42, 125.0, &target));

    const auto result = (*torun)();

    CHECK(result == 14);
    CHECK(target == 42 + static_cast<int>(125.0));

    target = -12;

    CHECK((*torun)() == result);

    CHECK(target != -12);
    target = -12;

    auto thread1 = thread(bind(&TestChanger, 5, 72.0, &target));

    thread1.join();

    CHECK(target == 5 + 72.0);

    target = -12;

    // Test on one thread first //
    
    Functions.push_back(torun);

    ThreadRunner(QueueMutex, Functions);

    CHECK(target == 42 + 125.0);
    CHECK(Functions.size() == 0);

    target = -12;

    {
        std::lock_guard<std::mutex> locked(QueueMutex);

        Functions.push_back(torun);
    }

    auto runningthread = thread(bind(&ThreadRunner, ref(QueueMutex), ref(Functions)));

    runningthread.join();

    CHECK(target != -12);
    CHECK(target == 42 + 125.0);
    CHECK(Functions.size() == 0);

    int secondary = 250;

    {
        std::lock_guard<std::mutex> locked(QueueMutex);

        Functions.push_back(make_shared<function<int ()>>(bind(&TestChanger, 12, 12.0, &target)));
        Functions.push_back(make_shared<function<int ()>>(bind(
                    &TestChanger, 12, 25.4, &secondary)));
        
        Functions.push_back(torun);
    }

    auto runningthread2 = thread(bind(&ThreadRunner, ref(QueueMutex), ref(Functions)));

    runningthread2.join();

    CHECK(target == 12 + 12.0);
    CHECK(secondary == (int)(12 + 25.4));
    CHECK(Functions.size() == 0);
}

TEST_CASE("Modulo '%' operator expected results", "[std]") {

    CHECK(0 % 8 == 0);
    CHECK(1 % 8 == 1);
    CHECK(2 % 8 == 2);
    CHECK(3 % 8 == 3);
    CHECK(4 % 8 == 4);
    CHECK(5 % 8 == 5);
    CHECK(6 % 8 == 6);
    CHECK(7 % 8 == 7);
    CHECK(8 % 8 == 0);
    CHECK(9 % 8 == 1);

}

TEST_CASE("std::copy with vectors", "[std]"){

    std::vector<uint8_t> dataVector = {1, 2, 3, 4, 5, 6, 7, 8};

    constexpr size_t leftSize = 2;

    std::vector<uint8_t> newData;
    newData.resize(dataVector.size() - leftSize);

    CHECK(newData.size() == dataVector.size() - leftSize);    

    std::copy(dataVector.begin() + leftSize, dataVector.end(),
        newData.begin());

    CHECK(newData.size() == dataVector.size() - leftSize);

    CHECK(newData == std::vector<uint8_t>({3, 4, 5, 6, 7, 8}));
}

TEST_CASE("std::abs", "[std]"){

    CHECK(std::abs(1.0f - 1.0f) == 0.f);
}

