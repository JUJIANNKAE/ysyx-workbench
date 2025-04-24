module top (
    input  wire        clk,
    input  wire        rst_n,
    input  wire [ 1:0] sw,
    output wire [15:0] ledr
);

    assign ledr[0] = sw[0] ^ sw[1];

    light u_light (
        .clk  (clk),
        .rst_n(rst_n),
        .ledr (ledr[15:1])
    );

endmodule
