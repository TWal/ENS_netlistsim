#include "JitSimulator.h"
#include <cassert>

JitSimulator::JitSimulator(const Netlist& ns) :
    Simulator(ns) {

    asmjit::X86Assembler assemb(&_runtime);
    asmjit::X86Compiler comp(&assemb);

    //asmjit::FileLogger logger(stdout);
    //assemb.setLogger(&logger);

    comp.addFunc(asmjit::FuncBuilder2<void, size_t*, size_t*>(asmjit::kCallConvHost));
    asmjit::X86GpVar vars = comp.newIntPtr("vars");
    asmjit::X86GpVar oldVars = comp.newIntPtr("oldVars");
    asmjit::X86GpVar tmp = comp.newInt64("tmp");
    asmjit::X86GpVar tmpMux = comp.newInt64("tmpMux");
    comp.setArg(0, vars);
    comp.setArg(1, oldVars);

    auto varToPtr = [&](size_t id) {
        return asmjit::x86::ptr(vars, id*sizeof(size_t), 1);
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
                comp.mov(tmp, asmjit::x86::ptr(oldVars, c.args[0]*sizeof(size_t), 1));
                comp.mov(varToPtr(c.varId), tmp);
                break;
            case OP_RAM:
                //TODO
                break;
            case OP_ROM:
                //TODO
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
                comp.and_(tmp, (1 << (c.args[1]-c.args[0]+1))-1);
                comp.mov(varToPtr(c.varId), tmp);
                //_vars[c.varId] = (_vars[c.args[2]] >> c.args[0]) & ((1 << (c.args[1]-c.args[0]+1))-1);
                break;
            case OP_CONCAT:
                comp.mov(tmp, varToPtr(c.args[0]));
                comp.shl(tmp, _ns.nappeSizes[c.args[1]]);
                comp.or_(tmp, varToPtr(c.args[1]));
                comp.mov(varToPtr(c.varId), tmp);
                //_vars[c.varId] = (_vars[c.args[0]] << _ns.nappeSizes[c.args[1]]) | _vars[c.args[1]];
                break;
        }
    }

    comp.endFunc();
    comp.finalize();
    // The prototype of the generated function.
    _func = asmjit_cast<FuncType>(assemb.make());
}

JitSimulator::~JitSimulator() {
    _runtime.release((void*)_func);
}

void JitSimulator::simulate() {
    _func(_curVars->data(), _curOldVars->data());
    endSimulation();
}

