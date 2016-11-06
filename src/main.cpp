#include "Parser.h"
#include "BasicSimulator.h"
#include "JitSimulator.h"
#include "Utils.h"

#include <fstream>
#include <streambuf>
#include <string>
#include <getopt.h>

std::string readFile(const std::string& file) {
    std::ifstream t(file);
    std::string s((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    return s;
}

void readInput(const Netlist& ns, Simulator* sim) {
    for(size_t i = 0; i < ns.input.size(); ++i) {
        if(feof(stdin)) {
            exit(0);
        }
        printf("%s = ", ns.idToName[ns.input[i]].c_str());
        fflush(stdout);
        size_t cur;
        scanf("%lu", &cur);
        sim->setInput(i, cur);
    }
}

void printOutput(const Netlist& ns, Simulator* sim) {
    for(size_t i = 0; i < ns.output.size(); ++i) {
        size_t maskedOutput = masknbit(sim->getOutput(i), ns.nappeSizes[ns.output[i]]);
        printf("%s = %lu\n", ns.idToName[ns.output[i]].c_str(), maskedOutput);
    }
    printf("\n");
}

bool has(const std::string& str, char c) {
    return str.find_first_of(std::string(&c, 1)) != std::string::npos;
}

int main(int argc, char** argv) {
    struct {
        std::string opts;
        std::string inputFile;
        std::string rom;
        size_t ramSize;
        int nbIter;
    } opts = {"", "", "", 0, -1};
    while(true) {
        static option longOptions[] = {
            {"input", required_argument, 0, 'i'},
            {"rom", required_argument, 0, 'r'},
            {"ramsize", required_argument, 0, 's'},
            {"jit", no_argument, 0, 'j'}
        };
        int optionIndex = 0;
        int c = getopt_long(argc, argv, "i:r:j", longOptions, &optionIndex);
        if(c == -1) {
            break;
        }
        opts.opts.push_back(c);
        switch(c) {
            case 'i':
                opts.inputFile = optarg;
                break;
            case 'r':
                opts.rom = optarg;
                break;
            default:
                break;
        }
    }

    if(!has(opts.opts, 'i')) {
        printf("No input netlist given!\n");
        return 1;
    }
    std::string prog = readFile(opts.inputFile);
    Netlist ns = Parser::parse(prog);
    Parser::typeCheck(ns);

    std::string rom;
    if(has(opts.opts, 'r')) {
        rom = readFile(opts.rom);
    }

    Simulator* sim;
    if(has(opts.opts, 'j')) {
        sim = new JitSimulator(ns, rom);
    } else {
        sim = new BasicSimulator(ns, rom);
    }

    while(true) {
        readInput(ns, sim);
        sim->simulate();
        printOutput(ns, sim);
    }

    delete sim;
    return 0;
}
