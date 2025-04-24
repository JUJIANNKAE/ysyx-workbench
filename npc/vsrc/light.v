module light (
    input  wire        clk,
    input  wire        rst_n,
    output wire [14:0] ledr
);

    reg [14:0] ledr_r;
    reg [31:0] cnt;

    assign ledr = ledr_r;

    always @(posedge clk) begin
        if (!rst_n) begin
            ledr_r <= 1;
            cnt <= 0;
        end else if (cnt == 5_000_000) begin
            ledr_r <= {ledr_r[13:0], ledr_r[14]};
            cnt <= 0;
        end else begin
            ledr_r <= ledr_r;
            cnt <= cnt + 1;
        end
    end

endmodule
