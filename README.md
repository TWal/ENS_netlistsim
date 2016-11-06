NetlistSim
==========

Netlist simulator, a class project for the "Système digital" course at the ENS Ulm

How to compile
--------------

Clone this repository:

    git clone git@github.com:TWal/ENS_netlistsim.git netlistsim
    cd netlistsim

Clone lexertl in the directory `thirdparty`

    mkdir thirdparty && cd thirdparty
    git clone git@github.com:BenHanson/lexertl.git
    cd ..

Install [asmjit](https://github.com/asmjit/asmjit)

Build

    cmake .
    make -j8

Done!
