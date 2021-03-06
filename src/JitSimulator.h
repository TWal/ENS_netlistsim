#ifndef JITSIMULATOR_H
#define JITSIMULATOR_H

#include "Simulator.h"
#include <asmjit/asmjit.h>

class JitSimulator : public Simulator {
    public:
        JitSimulator(const Netlist& ns, const std::string& rom = "", size_t ramSize = 0);
        virtual ~JitSimulator();
        virtual void simulate();

    protected:
        asmjit::JitRuntime _runtime;
        typedef void (*FuncType)(size_t*, size_t*, const char*, char*);
        FuncType _func;
};

#endif

