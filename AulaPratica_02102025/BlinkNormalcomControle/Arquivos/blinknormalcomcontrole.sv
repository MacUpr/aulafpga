// ====================================================================
// top.sv — Blink + três grupos de q com frequências configuráveis
// q[8:6]  → mesma freq. e fase do led
// q[5:3]  → frequência 2 (parâmetros G2_*)
// q[2:0]  → frequência 3 (parâmetros G3_*)
// ====================================================================
module top #(
    parameter int unsigned F_CLK_HZ   = 25_000_000,

    // Grupo 1 (led e q[8:6])
    parameter int unsigned ON_MS      = 500,
    parameter int unsigned OFF_MS     = 500,

    // Grupo 2 (q[5:3]) — defina como quiser
    parameter int unsigned G2_ON_MS   = 200,
    parameter int unsigned G2_OFF_MS  = 800,

    // Grupo 3 (q[2:0]) — defina como quiser
    parameter int unsigned G3_ON_MS   = 100,
    parameter int unsigned G3_OFF_MS  = 100
)(
    input  logic clk,
    output logic led,
    output logic [8:0] q
);
    // --------- util: ms -> ticks ---------
    localparam int unsigned TICKS_PER_MS = (F_CLK_HZ/1000);

    // --------- Grupo 1 (blink + q[8:6]) ---------
    localparam int unsigned G1_ON_TKS    = TICKS_PER_MS * ON_MS;
    localparam int unsigned G1_OFF_TKS   = TICKS_PER_MS * OFF_MS;
    localparam int unsigned G1_PERIOD    = G1_ON_TKS + G1_OFF_TKS;
    localparam int unsigned G1_W         = (G1_PERIOD > 1) ? $clog2(G1_PERIOD) : 1;

    logic [G1_W-1:0] g1_cnt = '0;
    always_ff @(posedge clk) begin
        g1_cnt <= (g1_cnt == G1_PERIOD-1) ? '0 : (g1_cnt + 1'b1);
    end

    wire g1_wave = (g1_cnt < G1_ON_TKS);   // alto por ON, baixo por OFF
    assign led   = g1_wave;
    assign q[8:6] = {3{g1_wave}};          // mesma frequência E fase do LED

    // --------- Grupo 2 (q[5:3]) ---------
    localparam int unsigned G2_ON_TKS    = TICKS_PER_MS * G2_ON_MS;
    localparam int unsigned G2_OFF_TKS   = TICKS_PER_MS * G2_OFF_MS;
    localparam int unsigned G2_PERIOD    = G2_ON_TKS + G2_OFF_TKS;
    localparam int unsigned G2_W         = (G2_PERIOD > 1) ? $clog2(G2_PERIOD) : 1;

    logic [G2_W-1:0] g2_cnt = '0;
    always_ff @(posedge clk) begin
        // Se quiser manter fase fixa com o blink, você pode sincronizar quando g1 reseta:
        // if (g1_cnt == '0) g2_cnt <= '0; else ...
        g2_cnt <= (g2_cnt == G2_PERIOD-1) ? '0 : (g2_cnt + 1'b1);
    end

    wire g2_wave = (g2_cnt < G2_ON_TKS);
    assign q[5:3] = {3{g2_wave}};

    // --------- Grupo 3 (q[2:0]) ---------
    localparam int unsigned G3_ON_TKS    = TICKS_PER_MS * G3_ON_MS;
    localparam int unsigned G3_OFF_TKS   = TICKS_PER_MS * G3_OFF_MS;
    localparam int unsigned G3_PERIOD    = G3_ON_TKS + G3_OFF_TKS;
    localparam int unsigned G3_W         = (G3_PERIOD > 1) ? $clog2(G3_PERIOD) : 1;

    logic [G3_W-1:0] g3_cnt = '0;
    always_ff @(posedge clk) begin
        // idem: para travar fase com o blink, zere em g1_cnt == 0
        g3_cnt <= (g3_cnt == G3_PERIOD-1) ? '0 : (g3_cnt + 1'b1);
    end

    wire g3_wave = (g3_cnt < G3_ON_TKS);
    assign q[2:0] = {3{g3_wave}};

endmodule

