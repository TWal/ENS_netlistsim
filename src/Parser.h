#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <unordered_map>

enum Operation {
    OP_OR, OP_XOR, OP_AND, OP_NAND, OP_NOT,
    OP_REG, OP_RAM, OP_ROM,
    OP_SELECT, OP_SLICE, OP_CONCAT,
    OP_NOP
};

struct Command {
    size_t varId = -1;
    size_t op = -1;
    std::vector<size_t> args; //Either variable id or number
};

struct Netlist {
    std::vector<Command> commands;
    std::vector<std::string> idToName;
    std::unordered_map<std::string, size_t> nameToId;
    std::vector<size_t> input;
    std::vector<size_t> output;
    std::vector<size_t> nappeSizes;
};

namespace Parser {

    Netlist parse(const std::string& s);
    void printNetlist(const Netlist& ns);
    void typeCheck(const Netlist& ns);

}

#endif

