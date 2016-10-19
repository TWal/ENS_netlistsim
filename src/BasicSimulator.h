#ifndef BASICSIMULATOR_H
#define BASICSIMULATOR_H

#include "Simulator.h"

class BasicSimulator : public Simulator {
    public:
        BasicSimulator(const Netlist& ns);
        virtual void simulate(const std::vector<size_t>& in);
};

#endif

