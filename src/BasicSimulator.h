#ifndef BASICSIMULATOR_H
#define BASICSIMULATOR_H

#include "Simulator.h"

class BasicSimulator : public Simulator {
    public:
        BasicSimulator(const Netlist& ns, const std::string& rom = "", size_t ramSize = 0);
        virtual void simulate();
    protected:
        std::vector<size_t> _ramInstrId;
};

#endif

