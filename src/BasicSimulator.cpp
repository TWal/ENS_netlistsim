#include "BasicSimulator.h"
#include "Utils.h"
#include <cassert>

BasicSimulator::BasicSimulator(const Netlist& ns, const std::string& rom, size_t ramSize) :
    Simulator(ns, rom, ramSize), _ramInstrId() {

    for(size_t i = 0; i < _ns.commands.size(); ++i) {
        if(_ns.commands[i].op == OP_RAM) {
            _ramInstrId.push_back(i);
        }
    }
}

void BasicSimulator::simulate() {
    size_t* vars = _curVars->data();
    size_t addr;
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
                //b = ROM ad w ra we wa d
                //b <- ram[ra:ra+w]
                addr = vars[c.args[2]];
                assert(addr%8 == 0);
                vars[c.varId] = *((size_t*)(&_ram[masknbit(addr, c.args[0])/8]));
                break;
            case OP_ROM:
                addr = vars[c.args[2]];
                assert(addr%8 == 0);
                vars[c.varId] = *((size_t*)(&_rom[masknbit(addr, _ns.nappeSizes[c.args[2]])/8]));
                break;
            case OP_MUX:
                vars[c.varId] = (vars[c.args[0]]&1)? vars[c.args[1]] : vars[c.args[2]];
                break;
            case OP_SELECT:
                vars[c.varId] = (vars[c.args[1]] >> c.args[0]) & 1;
                break;
            case OP_SLICE:
                vars[c.varId] = masknbit(vars[c.args[2]] >> c.args[0],c.args[1]-c.args[0]+1);
                break;
            case OP_CONCAT:
                vars[c.varId] = (vars[c.args[1]] << _ns.nappeSizes[c.args[0]]) | vars[c.args[0]];
                break;
        }
    }

    for(size_t i : _ramInstrId) {
        const Command& c = _ns.commands[i];
        //b = ROM ad w ra we wa d
        //ram[wa:wa+w] <- d if we = 1
        if(vars[c.args[3]]&1) {
            addr = vars[c.args[4]];
            *((size_t*)(&_ram[masknbit(addr, c.args[0])/8]))= vars[c.args[5]];
        }
    }
    endSimulation();
}

