#include "Simulator.h"
#include <functional>
#include <cassert>

Simulator::Simulator(const Netlist& ns) :
    _ns(ns), _vars(ns.idToName.size(), 0) {
    std::vector<std::vector<size_t>> adjList(_ns.idToName.size(), std::vector<size_t>());;
    for(const Command& c : _ns.commands) {
        switch(c.op) {
            case OP_OR:
            case OP_XOR:
            case OP_AND:
            case OP_NAND:
                adjList[c.varId] = {{ c.args[0], c.args[1] }};
                break;
            case OP_REG:
                break;
            case OP_RAM:
                //TODO
                break;
            case OP_ROM:
                //TODO
                break;
            case OP_SELECT:
                adjList[c.varId] = {{ c.args[1] }};
                break;
            case OP_SLICE:
                adjList[c.varId] = {{ c.args[2] }};
                break;
            case OP_CONCAT:
                adjList[c.varId] = {{ c.args[0], c.args[1] }};
                break;
        }
    }

    enum NodeState {
        NotVisited,
        BeingProcessed,
        Done
    };

    std::vector<NodeState> states(_ns.idToName.size(), NotVisited);
    std::vector<size_t> sorted;

    std::function<void(size_t)> topoSort;
    topoSort = [&](size_t i) {
        if(states[i] == BeingProcessed) {
            fprintf(stderr, "Not a DAG");
            exit(1);
        }
        if(states[i] == NotVisited) {
            states[i] = BeingProcessed;
            for(size_t j : adjList[i]) {
                topoSort(j);
            }
            states[i] = Done;
            sorted.push_back(i);
        }
    };
    for(size_t i = 0; i < _ns.idToName.size(); ++i) {
        topoSort(i);
    }

    std::vector<size_t> varToCommand(_ns.idToName.size(), ~0);
    for(size_t i = 0; i < _ns.commands.size(); ++i) {
        varToCommand[_ns.commands[i].varId] = i;
    }

    std::vector<Command> newCommands;
    for(size_t i : sorted) {
        if(varToCommand[i] != ~0) {
            newCommands.push_back(_ns.commands[varToCommand[i]]);
        }
    }
    _ns.commands = newCommands;
}

void Simulator::simulate(const std::vector<size_t>& in) {
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
                //TODO: see what is inclusive and exclusive
                _vars[c.varId] = (_vars[c.args[2]] >> c.args[0]) & ((1 << (c.args[1]-c.args[0]))-1);
                break;
            case OP_CONCAT:
                _vars[c.varId] = (_vars[c.args[0]] << _ns.nappeSizes[c.args[1]]) | _vars[c.args[1]];
                break;
        }
    }
}

std::vector<size_t> Simulator::getOutput() const {
    std::vector<size_t> res;
    for(size_t i : _ns.output) {
        res.push_back(_vars[i]);
    }
    return res;
}



