#ifndef BASICSIMULATOR_H
#define BASICSIMULATOR_H

#include "Simulator.h"

class BasicSimulator : public Simulator {
    public:
        BasicSimulator(const Netlist& ns, const std::string& rom = "");
        virtual void simulate();
};

#endif

