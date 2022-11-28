#include "sindataset.h"

#include <cmath>

namespace ImChart {

SinDataSet::SinDataSet() {
    hasErrors         = true;

    m_timer.interval  = std::chrono::milliseconds(40);
    m_timer.onTimeout = [this]() { update(); };
    m_timer.start();

    _xdata.resize(1e5);
    _ydata.resize(1e5);

    for (int i = 0; i < 1e5; ++i) {
        const float x = float(i) / 100.;
        _xdata[i]     = x;
        _ydata[i]     = std::sin(_offset + x);
    }

    _xPosErrors.resize(1e5, 0.3);
    _xNegErrors.resize(1e5, 0.3);
    _yPosErrors.resize(1e5, 0.2);
    _yNegErrors.resize(1e5, 0.1);
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
    return 1e5;
}

void SinDataSet::update() {
    _offset += 0.1;

    for (int i = 0; i < 1e5; ++i) {
        const float x = float(i) / 100.;
        _xdata[i]     = x;
        _ydata[i]     = std::sin(_offset + x);
    }

    dataChanged(0, getDataCount());
}

constexpr int SIZE = 2000;

SinDataSet2D::SinDataSet2D() {
    hasErrors         = true;

    m_timer.interval  = std::chrono::milliseconds(40);
    m_timer.onTimeout = [this]() { update(); };
    m_timer.start();

    _xdata.resize(SIZE);
    _ydata.resize(SIZE);
    _zdata.resize(SIZE*SIZE);

    for (int i = 0; i < SIZE; ++i) {
        const float x = float(i) / 100.;
        _xdata[i]     = x;
        for (int j = 0; j < SIZE; ++j) {
            const float y = float(j) / 100.;
            _ydata[j] = y;
            _zdata[i * SIZE + j]     = std::sin(_offset + x + y);
        }
    }
}

SinDataSet2D::~SinDataSet2D() {
}

float SinDataSet2D::get(int dimIndex, int index) const {
    return (dimIndex == 0 ? _xdata : dimIndex == 1 ? _ydata : _zdata)[index];
}

std::span<float> SinDataSet2D::getValues(int dimIndex) {
    return dimIndex == 0 ? _xdata : (dimIndex == 1 ? _ydata : _zdata);
}

int SinDataSet2D::getDataCount() const {
    return SIZE*SIZE;
}

void SinDataSet2D::update() {
    _offset += 0.1;

    // for (int i = 0; i < 1e5; ++i) {
    //     const float x = float(i) / 100.;
    //     _xdata[i]     = x;
    //     _ydata[i]     = std::sin(_offset + x);
    // }

    dataChanged(0, getDataCount());
}

} // namespace ImChart
