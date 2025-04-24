#include "Vtop.h"
#include <nvboard.h>

static Vtop dut;

void nvboard_bind_all_pins(Vtop *top);

static void single_cycle(Vtop *dut) {
    dut->clk = 0;
    dut->eval();
    dut->clk = 1;
    dut->eval();
}

static void reset(Vtop *dut, int n) {
    dut->rst_n = 0;
    while (n-- > 0)
        single_cycle(dut);
    dut->rst_n = 1;
}

int main() {
    nvboard_bind_all_pins(&dut);
    nvboard_init();

    reset(&dut, 10);

    while (1) {
        nvboard_update();
        single_cycle(&dut);
    }
}
