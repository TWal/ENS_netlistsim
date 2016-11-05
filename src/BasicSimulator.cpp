#include "BasicSimulator.h"
#include <cassert>

BasicSimulator::BasicSimulator(const Netlist& ns) :
    Simulator(ns) {
}

void BasicSimulator::simulate() {
    for(const Command& c : _ns.commands) {
        switch(c.op) {
            case OP_OR:
                (*_curVars)[c.varId] = (*_curVars)[c.args[0]] | (*_curVars)[c.args[1]];
                break;
            case OP_XOR:
                (*_curVars)[c.varId] = (*_curVars)[c.args[0]] ^ (*_curVars)[c.args[1]];
                break;
            case OP_AND:
                (*_curVars)[c.varId] = (*_curVars)[c.args[0]] & (*_curVars)[c.args[1]];
                break;
            case OP_NAND:
                (*_curVars)[c.varId] = ~((*_curVars)[c.args[0]] & (*_curVars)[c.args[1]]);
                break;
            case OP_NOT:
                (*_curVars)[c.varId] = ~((*_curVars)[c.args[0]]);
                break;
            case OP_REG:
                (*_curVars)[c.varId] = (*_curOldVars)[c.args[0]];
                break;
            case OP_RAM:
                //TODO
                break;
            case OP_ROM:
                //TODO
                break;
            case OP_MUX:
                (*_curVars)[c.varId] = ((*_curVars)[c.args[0]]&1)? (*_curVars)[c.args[1]] : (*_curVars)[c.args[2]];
                break;
            case OP_SELECT:
                (*_curVars)[c.varId] = ((*_curVars)[c.args[1]] >> c.args[0]) & 1;
                break;
            case OP_SLICE:
                (*_curVars)[c.varId] = ((*_curVars)[c.args[2]] >> c.args[0]) & ((1 << (c.args[1]-c.args[0]+1))-1);
                break;
            case OP_CONCAT:
                (*_curVars)[c.varId] = ((*_curVars)[c.args[0]] << _ns.nappeSizes[c.args[1]]) | (*_curVars)[c.args[1]];
                break;
        }
    }
    endSimulation();
}

