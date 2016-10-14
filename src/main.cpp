#include <lexertl/generator.hpp>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <lexertl/iterator.hpp>
#include <lexertl/lookup.hpp>

std::string prog =
    "INPUT a, b, c\n"
    "OUTPUT s, r\n"
    "VAR\n"
    "_l_1 : 4 , _l_3 , _l_4 , _l_5 , a, b, c, r, s\n"
    "IN\n"
    "r = OR _l_3 _l_5\n"
    "s = XOR _l_1 c\n"
    "_l_1 = XOR a b\n"
    "_l_3 = AND a b\n"
    "_l_4 = XOR a b\n"
    "_l_5 = AND _l_4 c\n"
;

int main() {
    enum TOKENS {
        SINPUT=1, SOUTPUT, SVAR, SIN,
        OOR, OXOR, OAND, ONAND,
        MREG, MRAM, MROM,
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
    rules.push("REG", MREG); //var
    rules.push("RAM", MRAM); //int int var var var var
    rules.push("ROM", MROM);
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

    lexertl::siterator iter(prog.begin(), prog.end(), sm);
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
            case MREG:
            case MRAM:
            case MROM:
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

    enum Operation {
        OP_OR, OP_XOR, OP_AND, OP_NAND,
        OP_REG, OP_RAM, OP_ROM,
        OP_SELECT, OP_SLICE, OP_CONCAT,
        OP_NOP
    };

    std::vector<std::string> idToName;
    std::unordered_map<std::string, size_t> nameToId;

    struct command {
        size_t varId = -1;
        size_t op = -1;
        std::vector<size_t> args; //Either variable id or number
    };

    auto getId = [&](const std::string& name) {
        auto it = nameToId.find(name);
        if(it == nameToId.end()) {
            nameToId[name] = idToName.size();
            idToName.push_back(name);
            return idToName.size()-1;
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
            case MREG:
                return OP_REG;
            case MRAM:
                return OP_RAM;
            case MROM:
                return OP_ROM;
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

    std::vector<command> commands;
    std::vector<size_t> input;
    std::vector<size_t> output;
    std::vector<size_t> nappeSizes;

    for(size_t i = 0; i < toktokvars.size(); ++i) {
        size_t currId;
        size_t currNappeSize = 0;
        if(toktokvars[i].size() == 1) {
            if(toktokvars[i][0].first != VARNAME) {
                fprintf(stderr, "Expected variable name...\n");
                exit(1);
            }
            currId = getId(toktokvars[i][0].second);
        } else if(toktokvars[i].size() == 3) {
            if(toktokvars[i][0].first != VARNAME || toktokvars[i][1].first != SEMICOLON || toktokvars[i][2].first != NUMBER) {
                fprintf(stderr, "Bad variable format...\n");
                exit(1);
            }
            currId = getId(toktokvars[i][0].second);
            currNappeSize = atoi(toktokvars[i][2].second.c_str());
        } else if(toktokvars[i].empty()) {
            continue;
        } else {
            fprintf(stderr, "Bad variable format...\n");
            exit(1);
        }

        if(varState[i] == SINPUT) {
            input.push_back(currId);
        } else if(varState[i] == SOUTPUT) {
            output.push_back(currId);
        }

        if(currId >= nappeSizes.size()) {
            assert(currId == nappeSizes.size());
            nappeSizes.push_back(currNappeSize);
        }
        nappeSizes[currId] = currNappeSize;
    }

    for(auto v : toktokin) {
        if(v.empty()) {
            continue;
        }
        command currentCommand;
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
                //???
                assert(0);
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
        commands.push_back(currentCommand);
    }

    printf("INPUT: ");
    for(size_t i : input) {
        printf("%s, ", idToName[i].c_str());
    }
    printf("\nOUTPUT: ");
    for(size_t i : output) {
        printf("%s, ", idToName[i].c_str());
    }
    printf("\nVARS: ");
    for(size_t i = 0; i < idToName.size(); ++i) {
        printf("(%s : %d), ", idToName[i].c_str(), nappeSizes[i]);
    }

    printf("\nIN\n");

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
            case OP_REG:
                return "REG";
            case OP_RAM:
                return "RAM";
            case OP_ROM:
                return "ROM";
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

    for(auto c : commands) {
        printf("%s = %s ", idToName[c.varId].c_str(), opToString((Operation)c.op));
        switch(c.op) {
            case OP_OR:
            case OP_XOR:
            case OP_AND:
            case OP_NAND:
            case OP_CONCAT:
                printf("%s %s\n", idToName[c.args[0]].c_str(), idToName[c.args[1]].c_str());
                break;
            case OP_REG:
                printf("%s\n", idToName[c.args[0]].c_str());
                break;
            case OP_RAM:
                printf("%d %d %s %s %s %s\n", c.args[0], c.args[1], idToName[c.args[2]].c_str(), idToName[c.args[3]].c_str(), idToName[c.args[4]].c_str(), idToName[c.args[5]].c_str());
                break;
            case OP_ROM:
                //???
                assert(0);
                break;
            case OP_SELECT:
                printf("%d %s\n", c.args[0], idToName[c.args[1]].c_str());
                break;
            case OP_SLICE:
                printf("%d %d %s\n", c.args[0], c.args[1], idToName[c.args[2]].c_str());
                break;
        }
    }

    return 0;
}
