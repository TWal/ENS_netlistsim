#include "BasicSimulator.h"
#include <cassert>

BasicSimulator::BasicSimulator(const Netlist& ns) :
    Simulator(ns) {
}

void BasicSimulator::simulate() {
    size_t* vars = _curVars->data();
    for(const Command& c : _ns.commands) {
        switch(c.op) {
            case OP_OR:
                vars[c.varId] = vars[c.args[0]] | vars[c.args[1]];
                break;
            case OP_XOR:
                vars[c.varId] = vars[c.args[0]] ^ vars[c.args[1]];
                break;
            case OP_AND:
                vars[c.varId] = vars[c.args[0]] & vars[c.args[1]];
                break;
            case OP_NAND:
                vars[c.varId] = ~(vars[c.args[0]] & vars[c.args[1]]);
                break;
            case OP_NOT:
                vars[c.varId] = ~(vars[c.args[0]]);
                break;
            case OP_REG:
                vars[c.varId] = (*_curOldVars)[c.args[0]];
                break;
            case OP_RAM:
                //TODO
                break;
            case OP_ROM:
                //TODO
                break;
            case OP_MUX:
                vars[c.varId] = (vars[c.args[0]]&1)? vars[c.args[1]] : vars[c.args[2]];
                break;
            case OP_SELECT:
                vars[c.varId] = (vars[c.args[1]] >> c.args[0]) & 1;
                break;
            case OP_SLICE:
                vars[c.varId] = (vars[c.args[2]] >> c.args[0]) & ((1 << (c.args[1]-c.args[0]+1))-1);
                break;
            case OP_CONCAT:
                vars[c.varId] = (vars[c.args[1]] << _ns.nappeSizes[c.args[0]]) | vars[c.args[0]];
                break;
        }
    }
    endSimulation();
}

