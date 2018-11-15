#pragma once

#include <mutex>

// Monitor class, courtesy of stack overflow.
// Original link: https://stackoverflow.com/a/48408987

template<class T>
class Monitor {
public:
    template<typename... Args>
    Monitor(Args&&... args) : m_cl(std::forward<Args>(args)...)
    {}

    struct MonitorHelper {
        MonitorHelper(Monitor* mon) : m_mon(mon), m_ul(mon->m_lock) {}
        T* operator->()
        {
            return &m_mon->m_cl;
        }
        Monitor* m_mon;
        std::unique_lock<std::mutex> m_ul;
    };

    MonitorHelper operator->()
    {
        return MonitorHelper(this);
    }
    MonitorHelper ManuallyLock()
    {
        return MonitorHelper(this);
    }
    T& GetThreadUnsafeAccess()
    {
        return m_cl;
    }

private:
    T m_cl;
    std::mutex m_lock;
};
