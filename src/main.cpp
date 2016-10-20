#include "Parser.h"
#include "BasicSimulator.h"
#include "JitSimulator.h"

//std::string prog =
    //"INPUT a, b, c\n"
    //"OUTPUT s, r\n"
    //"VAR\n"
    //"_l_1 , _l_3 , _l_4 , _l_5 , a, b, c, r, s\n"
    //"IN\n"
    //"r = OR _l_3 _l_5\n"
    //"s = XOR _l_1 c\n"
    //"_l_1 = XOR a b\n"
    //"_l_3 = AND a b\n"
    //"_l_4 = XOR a b\n"
    //"_l_5 = AND _l_4 c\n"
//;

std::string prog =
	"INPUT \n"
	"OUTPUT o\n"
	"VAR\n"
	"_l_2, c, o\n"
	"IN\n"
	"c = NOT _l_2\n"
	"o = REG c\n"
	"_l_2 = REG o\n"
;

int main() {
    Netlist ns = Parser::parse(prog);
    Parser::typeCheck(ns);
    //BasicSimulator sim(ns);
    JitSimulator sim(ns);
    //while(true) {
    for(size_t i = 0; i < 10; ++i) {
    //for(size_t i = 0; i < 10000000; ++i) {
        std::vector<size_t> in;
        for(size_t i : ns.input) {
            if(feof(stdin)) {
                return 0;
            }
            printf("%s = ", ns.idToName[i].c_str());
            fflush(stdout);
            size_t cur;
            scanf("%d", &cur);
            in.push_back(cur);
        }
        sim.simulate(in);
        for(size_t i = 0; i < ns.output.size(); ++i) {
            printf("%s = %d\n", ns.idToName[ns.output[i]].c_str(), sim.getOutput(i));
        }
        printf("\n");
    }
    return 0;
}
