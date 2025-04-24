#include "Vtop.h"
#include "Vtop___024root.h"
#include <cstdlib>
#include <iostream>
#include <stdlib.h>
#include <verilated.h>
#include <verilated_vcd_c.h>

#define MAX_SIM_TIME 20
vluint64_t sim_time = 0;

int main(int argc, char **argv, char **env) {
    Vtop *dut = new Vtop;

    Verilated::traceEverOn(true);
    VerilatedVcdC *m_trace = new VerilatedVcdC;
    dut->trace(m_trace, 5);
    m_trace->open("./build/obj_dir/waveform.vcd");

    while (sim_time < MAX_SIM_TIME) {
        int sw0 = rand() & 1;
        int sw1 = rand() & 1;

        dut->sw = (sw1 << 1) | sw0;
        dut->eval();

        printf("SW0 = %d, SW1 = %d, LD0 = %d\n", sw0, sw1, dut->ledr);
        assert(dut->ledr == (sw0 ^ sw1));

        m_trace->dump(sim_time);
        sim_time++;
    }

    m_trace->close();
    delete dut;
    exit(EXIT_SUCCESS);
}
