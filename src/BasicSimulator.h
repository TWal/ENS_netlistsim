#ifndef BASICSIMULATOR_H
#define BASICSIMULATOR_H

#include "Simulator.h"

class BasicSimulator : public Simulator {
    public:
        BasicSimulator(const Netlist& ns);
        virtual void simulate();
};

#endif

