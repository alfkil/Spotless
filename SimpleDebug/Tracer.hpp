#ifndef TRACER_HPP
#define TRACER_HPP

#include <proto/dos.h>
#include "Breaks.hpp"
#include <stdint.h>

class Tracer {
private:
    Breaks breaks;
    Process *process;
    ExceptionContext *context;
private:
    uint32_t branch();

    void setTraceBit();
    void unsetTraceBit();
    static bool hasTraceBit();
public:
    Tracer(Process *process, ExceptionContext *context);

    void activate(bool branching = true);
    void suspend();
};
#endif