#include "BasicSimulator.h"
#include <cassert>

BasicSimulator::BasicSimulator(const Netlist& ns) :
    Simulator(ns) {
}

void BasicSimulator::simulate(const std::vector<size_t>& in) {
    std::vector<size_t> oldVars = _vars;
    assert(_ns.input.size() == in.size());
    for(size_t i = 0; i < in.size(); ++i) {
        _vars[_ns.input[i]] = in[i];
    }

    for(const Command& c : _ns.commands) {
        switch(c.op) {
            case OP_OR:
                _vars[c.varId] = _vars[c.args[0]] | _vars[c.args[1]];
                break;
            case OP_XOR:
                _vars[c.varId] = _vars[c.args[0]] ^ _vars[c.args[1]];
                break;
            case OP_AND:
                _vars[c.varId] = _vars[c.args[0]] & _vars[c.args[1]];
                break;
            case OP_NAND:
                _vars[c.varId] = ~(_vars[c.args[0]] & _vars[c.args[1]]);
                break;
            case OP_NOT:
                _vars[c.varId] = ~(_vars[c.args[0]]);
                break;
            case OP_REG:
                _vars[c.varId] = oldVars[c.args[0]];
                break;
            case OP_RAM:
                //TODO
                break;
            case OP_ROM:
                //TODO
                break;
            case OP_SELECT:
                _vars[c.varId] = (_vars[c.args[1]] >> c.args[0]) & 1;
                break;
            case OP_SLICE:
                _vars[c.varId] = (_vars[c.args[2]] >> c.args[0]) & ((1 << (c.args[1]-c.args[0]+1))-1);
                break;
            case OP_CONCAT:
                _vars[c.varId] = (_vars[c.args[0]] << _ns.nappeSizes[c.args[1]]) | _vars[c.args[1]];
                break;
        }
    }
}

