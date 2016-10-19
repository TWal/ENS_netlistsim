#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "Parser.h"

class Simulator {
    public:
        Simulator(const Netlist& ns);
        virtual void simulate(const std::vector<size_t>& in) = 0;
        inline size_t getOutput(size_t i) {
            return _vars[_ns.output[i]];
        }

    protected:
        Netlist _ns;
        std::vector<size_t> _vars;
};

#endif

