#include "timer.h"

#include "backends/backend.h"

namespace ImChart {

void Timer::start() {
    Backend::instance().startTimer(this);
}

} // namespace ImChart
