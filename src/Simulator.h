#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "Parser.h"

class Simulator {
    public:
        Simulator(const Netlist& ns);
        virtual void simulate() = 0;
        inline void setInput(size_t i, size_t val) {
            (*_curVars)[_ns.input[i]] = val;
        }
        inline size_t getOutput(size_t i) {
            return (*_curOldVars)[_ns.output[i]];
        }

    protected:
        void endSimulation();
        Netlist _ns;
        std::vector<size_t> _vars;
        std::vector<size_t> _oldVars;
        std::vector<size_t>* _curVars;
        std::vector<size_t>* _curOldVars;
};

#endif

