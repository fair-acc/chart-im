#pragma once

#include <chrono>
#include <functional>

namespace ImChart {

class Timer {
public:
    void                      start();

    std::chrono::milliseconds interval = {};
    std::function<void()>     onTimeout;

private:
    bool m_running = false;
};

} // namespace ImChart
