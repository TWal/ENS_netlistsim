#include "JitSimulator.h"
#include "Utils.h"
#include <cassert>

JitSimulator::JitSimulator(const Netlist& ns, const std::string& rom_, size_t ramSize) :
    Simulator(ns, rom_, ramSize) {

    asmjit::X86Assembler assemb(&_runtime);
    asmjit::X86Compiler comp(&assemb);

    //asmjit::FileLogger logger(stdout);
    //assemb.setLogger(&logger);

    comp.addFunc(asmjit::FuncBuilder4<void, size_t*, size_t*, const char*, char*>(asmjit::kCallConvHost));
    asmjit::X86GpVar vars = comp.newIntPtr("vars");
    asmjit::X86GpVar oldVars = comp.newIntPtr("oldVars");
    asmjit::X86GpVar rom = comp.newIntPtr("rom");
    asmjit::X86GpVar ram = comp.newIntPtr("ram");
    asmjit::X86GpVar tmp = comp.newInt64("tmp");
    asmjit::X86GpVar tmpMux = comp.newInt64("tmpMux");
    comp.setArg(0, vars);
    comp.setArg(1, oldVars);
    comp.setArg(2, rom);
    comp.setArg(3, ram);

    auto varToPtr = [&](size_t id) {
        return asmjit::x86::ptr(vars, id*sizeof(size_t), 0);
    };

    for(const Command& c : _ns.commands) {
        switch(c.op) {
            case OP_OR:
                comp.mov(tmp, varToPtr(c.args[0]));
                comp.or_(tmp, varToPtr(c.args[1]));
                comp.mov(varToPtr(c.varId), tmp);
                break;
            case OP_XOR:
                comp.mov(tmp, varToPtr(c.args[0]));
                comp.xor_(tmp, varToPtr(c.args[1]));
                comp.mov(varToPtr(c.varId), tmp);
                break;
            case OP_AND:
                comp.mov(tmp, varToPtr(c.args[0]));
                comp.and_(tmp, varToPtr(c.args[1]));
                comp.mov(varToPtr(c.varId), tmp);
                break;
            case OP_NAND:
                comp.mov(tmp, varToPtr(c.args[0]));
                comp.and_(tmp, varToPtr(c.args[1]));
                comp.not_(tmp);
                comp.mov(varToPtr(c.varId), tmp);
                break;
            case OP_NOT:
                comp.mov(tmp, varToPtr(c.args[0]));
                comp.not_(tmp);
                comp.mov(varToPtr(c.varId), tmp);
                break;
            case OP_REG:
                comp.mov(tmp, asmjit::x86::ptr(oldVars, c.args[0]*sizeof(size_t), 0));
                comp.mov(varToPtr(c.varId), tmp);
                break;
            case OP_RAM:
                //b = ROM ad w ra we wa d
                //b <- ram[ra:ra+w]
                //addr = vars[c.args[2]];
                //assert(addr%8 == 0);
                //vars[c.varId] = *((size_t*)(&_ram[masknbit(addr, c.args[0])/8]));
                comp.mov(tmp, masknbit(~0, ns.nappeSizes[c.args[2]]));
                comp.and_(tmp, varToPtr(c.args[2]));
                comp.shr(tmp, 3);
                comp.mov(tmp, asmjit::x86::ptr(ram, tmp, 0));
                comp.mov(varToPtr(c.varId), tmp);
                break;
            case OP_ROM:
                comp.mov(tmp, masknbit(~0, ns.nappeSizes[c.args[2]]));
                comp.and_(tmp, varToPtr(c.args[2]));
                comp.shr(tmp, 3);
                comp.mov(tmp, asmjit::x86::ptr(rom, tmp, 0));
                comp.mov(varToPtr(c.varId), tmp);
                break;
            case OP_MUX:
                comp.mov(tmpMux, varToPtr(c.args[0]));
                comp.mov(tmp, varToPtr(c.args[1]));
                comp.test(tmpMux, 1);
                comp.cmove(tmp, varToPtr(c.args[2]));
                comp.mov(varToPtr(c.varId), tmp);
                break;
            case OP_SELECT:
                comp.mov(tmp, varToPtr(c.args[1]));
                comp.shr(tmp, c.args[0]);
                comp.and_(tmp, 1);
                comp.mov(varToPtr(c.varId), tmp);
                //_vars[c.varId] = (_vars[c.args[1]] >> c.args[0]) & 1;
                break;
            case OP_SLICE:
                comp.mov(tmp, varToPtr(c.args[2]));
                comp.shr(tmp, c.args[0]);
                comp.and_(tmp, masknbit(~0, (c.args[1]-c.args[0]+1)));
                comp.mov(varToPtr(c.varId), tmp);
                //_vars[c.varId] = (_vars[c.args[2]] >> c.args[0]) & ((1 << (c.args[1]-c.args[0]+1))-1);
                break;
            case OP_CONCAT:
                comp.mov(tmp, varToPtr(c.args[1]));
                comp.shl(tmp, _ns.nappeSizes[c.args[0]]);
                comp.or_(tmp, varToPtr(c.args[0]));
                comp.mov(varToPtr(c.varId), tmp);
                //_vars[c.varId] = (_vars[c.args[1]] << _ns.nappeSizes[c.args[0]]) | _vars[c.args[0]];
                break;
        }
    }

    for(const Command& c : _ns.commands) {
        if(c.op == OP_RAM) {
            //b = ROM ad w ra we wa d
            //ram[wa:wa+w] <- d if we = 1
            asmjit::Label lab = comp.newLabel();
            comp.mov(tmp, varToPtr(c.args[3]));
            comp.test(tmp, 1);
            comp.je(lab);
            comp.mov(tmp, masknbit(~0, ns.nappeSizes[c.args[4]]));
            comp.and_(tmp, varToPtr(c.args[4]));
            comp.shr(tmp, 3);
            comp.mov(tmpMux, varToPtr(c.args[5]));
            comp.mov(asmjit::x86::ptr(ram, tmp, 0), tmpMux);
            comp.bind(lab);
        }
    }
    comp.endFunc();
    comp.finalize();
    _func = asmjit_cast<FuncType>(assemb.make());
}

JitSimulator::~JitSimulator() {
    _runtime.release((void*)_func);
}

void JitSimulator::simulate() {
    _func(_curVars->data(), _curOldVars->data(), _rom.data(), _ram.data());
    endSimulation();
}

