#include "Simulator.h"
#include <functional>

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
            case OP_NOT:
                adjList[c.varId] = {{ c.args[0] }};
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

