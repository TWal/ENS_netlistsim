#include "Parser.h"

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
    Netlist ns = Parser::parse(prog);
    Parser::printNetlist(ns);
    return 0;
}
