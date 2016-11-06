#include "Parser.h"
#include <lexertl/generator.hpp>
#include <lexertl/iterator.hpp>
#include <lexertl/lookup.hpp>

namespace Parser {

Netlist parse(const std::string& s) {
    enum TOKENS {
        SINPUT=1, SOUTPUT, SVAR, SIN,
        OOR, OXOR, OAND, ONAND, ONOT,
        MREG, MRAM, MROM, MMUX,
        NSELECT, NSLICE, NCONCAT,
        VARNAME, NUMBER,
        COMMA, SEMICOLON, EQUAL, NEWLINE
    };
    lexertl::rules rules;
    lexertl::state_machine sm;

    rules.push("INPUT", SINPUT);
    rules.push("OUTPUT", SOUTPUT);
    rules.push("VAR", SVAR);
    rules.push("IN", SIN);
    rules.push("OR", OOR); //var var
    rules.push("XOR", OXOR); //var var
    rules.push("AND", OAND); //var var
    rules.push("NAND", ONAND); //var var
    rules.push("NOT", ONOT); //var var
    rules.push("REG", MREG); //var
    rules.push("RAM", MRAM); //int int var var var var
    rules.push("ROM", MROM); //int int var
    rules.push("MUX", MMUX); //var var var
    rules.push("SELECT", NSELECT); //int var
    rules.push("SLICE", NSLICE); //int int var
    rules.push("CONCAT", NCONCAT); //var var
    rules.push("[_a-z][_a-z0-9]*", VARNAME);
    rules.push("[0-9]+", NUMBER);
    rules.push(",", COMMA);
    rules.push(":", SEMICOLON);
    rules.push("=", EQUAL);
    rules.push("\n", NEWLINE);
    rules.push("[ \t]+", rules.skip());
    lexertl::generator::build(rules, sm);

    std::vector<std::vector<std::pair<size_t, std::string>>> toktokvars, toktokin;
    std::vector<int> varState;

    lexertl::siterator iter(s.begin(), s.end(), sm);
    lexertl::siterator end;
    int currState = -1;

    //TODO: maybe use a real parser?
    for (; iter != end; ++iter) {
        switch(iter->id) {
            case SINPUT:
            case SOUTPUT:
            case SVAR:
            case SIN:
                currState = iter->id;
                // /!\ no break
            case COMMA:
            case NEWLINE:
                if(currState == SINPUT || currState == SVAR || currState == SOUTPUT) {
                    toktokvars.push_back(std::vector<std::pair<size_t, std::string>>());
                    varState.push_back(currState);
                } else if(currState == SIN) {
                    toktokin.push_back(std::vector<std::pair<size_t, std::string>>());
                }
                break;
            case OOR:
            case OXOR:
            case OAND:
            case ONAND:
            case ONOT:
            case MREG:
            case MRAM:
            case MROM:
            case MMUX:
            case NSELECT:
            case NSLICE:
            case NCONCAT:
                if(currState != SIN) {
                    fprintf(stderr, "'%s' may be used only after IN\n", iter->str().c_str());
                    exit(1);
                }
                toktokin.back().push_back(std::make_pair(iter->id, iter->str()));
                break;

            case VARNAME:
                if(currState == SINPUT || currState == SVAR || currState == SOUTPUT) {
                    toktokvars.back().push_back(std::make_pair(iter->id, iter->str()));
                } else if(currState == SIN) {
                    toktokin.back().push_back(std::make_pair(iter->id, iter->str()));
                } else {
                    fprintf(stderr, "INPUT, OUTPUT, VAR, IN was not specified before ...\n");
                    exit(1);
                }
                break;

            case SEMICOLON:
                if(currState == SVAR) {
                    toktokvars.back().push_back(std::make_pair(iter->id, iter->str()));
                } else {
                    fprintf(stderr, "':' may only be used after VAR\n");
                    exit(1);
                }
                break;

            case EQUAL:
                if(currState == SIN) {
                    toktokin.back().push_back(std::make_pair(iter->id, iter->str()));
                } else {
                    fprintf(stderr, "'=' may only be used after IN\n");
                }
                break;

            case NUMBER:
                if(currState == SVAR) {
                    toktokvars.back().push_back(std::make_pair(iter->id, iter->str()));
                } else if(currState == SIN) {
                    toktokin.back().push_back(std::make_pair(iter->id, iter->str()));
                } else {
                    fprintf(stderr, "Numbers  may only be used after IN, OUTPUT or VAR");
                }
            default:
                break;
        }
        //std::cout << "Id: " << iter->id << ", Token: '" << iter->str() <<"'\n";
    }

    Netlist ns;

    auto addGetId = [&](const std::string& name) {
        auto it = ns.nameToId.find(name);
        if(it == ns.nameToId.end()) {
            ns.nameToId[name] = ns.idToName.size();
            ns.idToName.push_back(name);
            return ns.idToName.size()-1;
        } else {
            return it->second;
        }
    };

    auto getId = [&](const std::string& name) {
        auto it = ns.nameToId.find(name);
        if(it == ns.nameToId.end()) {
            fprintf(stderr, "Unknown variable name: %s\n", name.c_str());
            exit(1);
        } else {
            return it->second;
        }
    };

    auto tokenToOperator = [](size_t tok) {
        switch(tok) {
            case OOR:
                return OP_OR;
            case OXOR:
                return OP_XOR;
            case OAND:
                return OP_AND;
            case ONAND:
                return OP_NAND;
            case ONOT:
                return OP_NOT;
            case MREG:
                return OP_REG;
            case MRAM:
                return OP_RAM;
            case MROM:
                return OP_ROM;
            case MMUX:
                return OP_MUX;
            case NSELECT:
                return OP_SELECT;
            case NSLICE:
                return OP_SLICE;
            case NCONCAT:
                return OP_CONCAT;
            default:
                return OP_NOP;
        }
    };

    for(size_t i = 0; i < toktokvars.size(); ++i) {
        size_t currId;
        size_t currNappeSize = 1;
        if(toktokvars[i].size() == 1) {
            if(toktokvars[i][0].first != VARNAME) {
                fprintf(stderr, "Expected variable name...\n");
                exit(1);
            }
            currId = addGetId(toktokvars[i][0].second);
        } else if(toktokvars[i].size() == 3) {
            if(toktokvars[i][0].first != VARNAME || toktokvars[i][1].first != SEMICOLON || toktokvars[i][2].first != NUMBER) {
                fprintf(stderr, "Bad variable format...\n");
                exit(1);
            }
            currId = addGetId(toktokvars[i][0].second);
            currNappeSize = atoi(toktokvars[i][2].second.c_str());
        } else if(toktokvars[i].empty()) {
            continue;
        } else {
            fprintf(stderr, "Bad variable format...\n");
            exit(1);
        }

        if(varState[i] == SINPUT) {
            ns.input.push_back(currId);
        } else if(varState[i] == SOUTPUT) {
            ns.output.push_back(currId);
        }

        if(currId >= ns.nappeSizes.size()) {
            assert(currId == ns.nappeSizes.size());
            ns.nappeSizes.push_back(currNappeSize);
        }
        ns.nappeSizes[currId] = currNappeSize;
    }

    for(auto v : toktokin) {
        if(v.empty()) {
            continue;
        }
        Command currentCommand;
        if(v.size() < 4) {
            fprintf(stderr, "Bad operation format 1...\n");
            exit(1);
        }
        Operation op = tokenToOperator(v[2].first);
        if(v[0].first != VARNAME || v[1].first != EQUAL || op == OP_NOP) {
            fprintf(stderr, "Bad operation format 2...\n");
            exit(1);
        }

        currentCommand.varId = getId(v[0].second);
        currentCommand.op = op;
        switch(op) {
            case OP_OR:
            case OP_XOR:
            case OP_AND:
            case OP_NAND:
            case OP_CONCAT:
                if(v.size() != 5 || v[3].first != VARNAME || v[4].first != VARNAME) {
                    fprintf(stderr, "Operation has not the right number of arguments...\n");
                    exit(1);
                }
                currentCommand.args.push_back(getId(v[3].second));
                currentCommand.args.push_back(getId(v[4].second));
                break;
            case OP_NOT:
                if(v.size() != 4 || v[3].first != VARNAME) {
                    fprintf(stderr, "Operation has not the right number of arguments...\n");
                    exit(1);
                }
                currentCommand.args.push_back(getId(v[3].second));
                break;
            case OP_REG:
                if(v.size() != 4 || v[3].first != VARNAME) {
                    fprintf(stderr, "Operation has not the right number of arguments...\n");
                    exit(1);
                }
                currentCommand.args.push_back(getId(v[3].second));
                break;
            case OP_RAM:
                if(v.size() != 9 || v[3].first != NUMBER || v[4].first != NUMBER || v[5].first != VARNAME
                                 || v[6].first != VARNAME || v[7].first != VARNAME || v[8].first != VARNAME) {
                    fprintf(stderr, "Operation has not the right number of arguments...\n");
                    exit(1);
                }
                currentCommand.args.push_back(atoi(v[3].second.c_str()));
                currentCommand.args.push_back(atoi(v[4].second.c_str()));
                for(int i = 5; i < 9; ++i) {
                    currentCommand.args.push_back(getId(v[i].second));
                }
                break;
            case OP_ROM:
                if(v.size() != 6 || v[3].first != NUMBER || v[4].first != NUMBER || v[5].first != VARNAME) {
                    fprintf(stderr, "Operation has not the right number of arguments...\n");
                    exit(1);
                }
                currentCommand.args.push_back(atoi(v[3].second.c_str()));
                currentCommand.args.push_back(atoi(v[4].second.c_str()));
                currentCommand.args.push_back(getId(v[5].second));
                break;
            case OP_MUX:
                if(v.size() != 6 || v[3].first != VARNAME || v[4].first != VARNAME || v[5].first != VARNAME) {
                    fprintf(stderr, "Operation has not the right number of arguments...\n");
                    exit(1);
                }
                currentCommand.args.push_back(getId(v[3].second));
                currentCommand.args.push_back(getId(v[4].second));
                currentCommand.args.push_back(getId(v[5].second));
                break;
            case OP_SELECT:
                if(v.size() != 5 || v[3].first != NUMBER || v[4].first != VARNAME) {
                    fprintf(stderr, "Operation has not the right number of arguments...\n");
                    exit(1);
                }
                currentCommand.args.push_back(atoi(v[3].second.c_str()));
                currentCommand.args.push_back(getId(v[4].second));
                break;
            case OP_SLICE:
                if(v.size() != 6 || v[3].first != NUMBER || v[4].first != NUMBER || v[5].first != VARNAME) {
                    fprintf(stderr, "Operation has not the right number of arguments...\n");
                    exit(1);
                }
                currentCommand.args.push_back(atoi(v[3].second.c_str()));
                currentCommand.args.push_back(atoi(v[4].second.c_str()));
                currentCommand.args.push_back(getId(v[5].second));
                break;
        }
        ns.commands.push_back(currentCommand);
    }
    return ns;
}



void printNetlist(const Netlist& ns) {
    printf("INPUT ");
    for(size_t i = 0; i < ns.input.size(); ++i) {
        printf("%s", ns.idToName[ns.input[i]].c_str());
        if(i < ns.input.size()-1) {
            printf(", ");
        }
    }
    printf("\n");

    printf("OUTPUT ");
    for(size_t i = 0; i < ns.output.size(); ++i) {
        printf("%s", ns.idToName[ns.output[i]].c_str());
        if(i < ns.output.size()-1) {
            printf(", ");
        }
    }
    printf("\n");

    printf("VAR\n\t");
    for(size_t i = 0; i < ns.idToName.size(); ++i) {
        if(ns.nappeSizes[i] > 1) {
            printf("%s : %d", ns.idToName[i].c_str(), ns.nappeSizes[i]);
        } else {
            printf("%s", ns.idToName[i].c_str());
        }
        if(i < ns.idToName.size()-1) {
            printf(", ");
        }
    }
    printf("\n");

    printf("IN\n");

    auto opToString = [](Operation op) {
        switch(op) {
            case OP_OR:
                return "OR";
            case OP_XOR:
                return "XOR";
            case OP_AND:
                return "AND";
            case OP_NAND:
                return "NAND";
            case OP_NOT:
                return "NOT";
            case OP_REG:
                return "REG";
            case OP_RAM:
                return "RAM";
            case OP_ROM:
                return "ROM";
            case OP_MUX:
                return "MUX";
            case OP_SELECT:
                return "SELECT";
            case OP_SLICE:
                return "SLICE";
            case OP_CONCAT:
                return "CONCAT";
            case OP_NOP:
                return "NOP";
        }
    };

    for(auto c : ns.commands) {
        printf("%s = %s ", ns.idToName[c.varId].c_str(), opToString((Operation)c.op));
        switch(c.op) {
            case OP_OR:
            case OP_XOR:
            case OP_AND:
            case OP_NAND:
            case OP_CONCAT:
                printf("%s %s\n", ns.idToName[c.args[0]].c_str(), ns.idToName[c.args[1]].c_str());
                break;
            case OP_NOT:
                printf("%s\n", ns.idToName[c.args[0]].c_str());
                break;
            case OP_REG:
                printf("%s\n", ns.idToName[c.args[0]].c_str());
                break;
            case OP_RAM:
                printf("%lu %lu %s %s %s %s\n", c.args[0], c.args[1], ns.idToName[c.args[2]].c_str(), ns.idToName[c.args[3]].c_str(), ns.idToName[c.args[4]].c_str(), ns.idToName[c.args[5]].c_str());
                break;
            case OP_ROM:
                printf("%lu %lu %s\n", c.args[0], c.args[1], ns.idToName[c.args[2]].c_str());
                break;
            case OP_MUX:
                printf("%s %s %s\n", ns.idToName[c.args[0]].c_str(), ns.idToName[c.args[1]].c_str(), ns.idToName[c.args[2]].c_str());
                break;
            case OP_SELECT:
                printf("%lu %s\n", c.args[0], ns.idToName[c.args[1]].c_str());
                break;
            case OP_SLICE:
                printf("%lu %lu %s\n", c.args[0], c.args[1], ns.idToName[c.args[2]].c_str());
                break;
        }
    }
}

void typeCheck(const Netlist& ns) {
    //TODO: check that no variables are assigned twice
    //TODO: check that all outputs are defined?
    for(const Command& c : ns.commands) {
        auto nappe = [&](size_t id) {
            return ns.nappeSizes[c.args[id]];
        };
        size_t varNappe = ns.nappeSizes[c.varId];
        switch(c.op) {
            case OP_OR:
            case OP_XOR:
            case OP_AND:
            case OP_NAND:
                assert(varNappe == nappe(0) && nappe(0) == nappe(1));
                break;
            case OP_NOT:
            case OP_REG:
                assert(varNappe == nappe(0));
                break;
            case OP_RAM:
                //TODO
                break;
            case OP_ROM:
                assert(c.args[0] == nappe(2) && varNappe == c.args[1]);
                break;
            case OP_MUX:
                assert(varNappe == nappe(1) && nappe(2) == nappe(1) && nappe(0) == 1);
                break;
            case OP_SELECT:
                assert(varNappe == 1 && nappe(1) > c.args[0]);
                break;
            case OP_SLICE:
                assert(varNappe == c.args[1] - c.args[0] + 1 && nappe(2) > c.args[1]);
                break;
            case OP_CONCAT:
                assert(varNappe == nappe(0)+nappe(1));
                break;
            case OP_NOP:
            default:
                assert(false);
                break;
        };
    }
}

}
