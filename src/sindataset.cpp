#include "sindataset.h"

#include <cmath>

namespace ImChart {

SinDataSet::SinDataSet() {
    hasErrors         = true;

    m_timer.interval  = std::chrono::milliseconds(40);
    m_timer.onTimeout = [this]() { update(); };
    m_timer.start();

    _xdata.resize(1e6);
    _ydata.resize(1e6);

    for (int i = 0; i < 1e6; ++i) {
        const float x = float(i) / 100.;
        _xdata[i]     = x;
        _ydata[i]     = std::sin(_offset + x);
    }

    _xPosErrors.resize(1e6, 0.3);
    _xNegErrors.resize(1e6, 0.3);
    _yPosErrors.resize(1e6, 0.2);
    _yNegErrors.resize(1e6, 0.1);
}

SinDataSet::~SinDataSet() {
}

float SinDataSet::get(int dimIndex, int index) const {
    return (dimIndex == 0 ? _xdata : _ydata)[index];
}

std::span<float> SinDataSet::getValues(int dimIndex) {
    return dimIndex == 0 ? _xdata : _ydata;
}

std::span<float> SinDataSet::getPositiveErrors(int dimIndex) {
    return dimIndex == 0 ? _xPosErrors : _yPosErrors;
}

std::span<float> SinDataSet::getNegativeErrors(int dimIndex) {
    return dimIndex == 0 ? _xNegErrors : _yNegErrors;
}

int SinDataSet::getDataCount() const {
    return 1e6;
}

void SinDataSet::update() {
    // _offset += 0.1;
    //
    // for (int i = 0; i < 1e6; ++i) {
    //     const float x = float(i) / 100.;
    //     _xdata[i]     = x;
    //     _ydata[i]     = std::sin(_offset + x);
    // }

    dataChanged(0, getDataCount());
}

} // namespace ImChart
