#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "Parser.h"

class Simulator {
    public:
        Simulator(const Netlist& ns);
        void simulate(const std::vector<size_t>& in);
        std::vector<size_t> getOutput() const;

    private:
        Netlist _ns;
        std::vector<size_t> _vars;
};

#endif

