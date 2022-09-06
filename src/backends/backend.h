#pragma once

#include <memory>

#include "utils.h"

namespace ImChart {

class Window;

namespace Backend {

class Window;

class Backend {
public:
    virtual ~Backend()                                                                  = default;

    virtual void                    run()                                               = 0;

    virtual void                   *nativeDisplay()                                     = 0;

    virtual std::unique_ptr<Window> createWindow(ImChart::Window *window, int w, int h) = 0;

    virtual void                    scheduleRender(ImChart::Window *window)             = 0;
};

class Window {
public:
    virtual ~Window()                   = default;

    virtual void *nativeWindow()        = 0;

    virtual void  show()                = 0;
    virtual void  setSize(int w, int h) = 0;

    virtual Size  pixelSize() const     = 0;
};

bool     create();
Backend &instance();

} // namespace Backend

} // namespace ImChart
