module top (
    input  wire       clk,
    input  wire       rst,
    input  wire [1:0] sw,
    output wire       ledr
);

    assign ledr = sw[0] ^ sw[1];

endmodule
