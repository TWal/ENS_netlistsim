#ifndef JITSIMULATOR_H
#define JITSIMULATOR_H

#include "Simulator.h"
#include <asmjit/asmjit.h>

class JitSimulator : public Simulator {
    public:
        JitSimulator(const Netlist& ns);
        ~JitSimulator();
        virtual void simulate(const std::vector<size_t>& in);

    protected:
        asmjit::JitRuntime _runtime;
        typedef void (*FuncType)(size_t*, size_t*);
        FuncType _func;
};

#endif

