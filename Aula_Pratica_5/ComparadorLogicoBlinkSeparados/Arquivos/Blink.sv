// Blink.sv — Blink ON/OFF em ms
module Blink #(
    parameter int unsigned F_CLK_HZ = 25_000_000,
    parameter int unsigned ON_MS    = 250,
    parameter int unsigned OFF_MS   = 250
) (
    input  logic clk,
    output logic led
);
    localparam int unsigned ON_TICKS   = (F_CLK_HZ / 1000) * ON_MS;
    localparam int unsigned OFF_TICKS  = (F_CLK_HZ / 1000) * OFF_MS;
    localparam int unsigned PERIOD_TKS = ON_TICKS + OFF_TICKS;

    localparam int unsigned CNT_W = (PERIOD_TKS > 1) ? $clog2(PERIOD_TKS) : 1;

    logic [CNT_W-1:0] cnt = '0;

    always_ff @(posedge clk) begin
        if (cnt == PERIOD_TKS - 1)
            cnt <= '0;
        else
            cnt <= cnt + 1'b1;
    end

    assign led = (cnt < ON_TICKS);
endmodule
