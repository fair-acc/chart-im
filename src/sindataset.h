#pragma once

#include <vector>

#include "timer.h"
#include <dataset.h>

namespace ImChart {

class Timer;

class SinDataSet : public DataSet {
public:
    SinDataSet();
    ~SinDataSet();

    float            get(int dimIndex, int index) const final;
    int              getDataCount() const final;
    int              getDimension() const final { return 2; }
    std::span<float> getValues(int dimIndex) final;

    std::span<float> getPositiveErrors(int dimIndex) final;
    std::span<float> getNegativeErrors(int dimIndex) final;

    void             update();

private:
    double             _offset = 0;
    std::vector<float> _xdata;
    std::vector<float> _ydata;
    std::vector<float> _xPosErrors;
    std::vector<float> _xNegErrors;
    std::vector<float> _yPosErrors;
    std::vector<float> _yNegErrors;
    Timer              m_timer;
};


class SinDataSet2D : public DataSet {
public:
    SinDataSet2D();
    ~SinDataSet2D();

    float            get(int dimIndex, int index) const final;
    int              getDataCount() const final;
    int              getDimension() const final { return 3; }
    std::span<float> getValues(int dimIndex) final;

    void             update();

private:
    double             _offset = 0;
    std::vector<float> _xdata;
    std::vector<float> _ydata;
    std::vector<float> _zdata;
    Timer              m_timer;
};

} // namespace ImChart
