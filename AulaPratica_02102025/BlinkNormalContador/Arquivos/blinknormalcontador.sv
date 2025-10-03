// ====================================================================
// top.sv — Blink + três contadores de 3 bits em q
// q[8:6] → contador avança na MESMA frequência do blink (mesmo período)
// q[5:3] → contador com período configurável (G2_*)
// q[2:0] → contador com período configurável (G3_*)
// ====================================================================
module top #(
    parameter int unsigned F_CLK_HZ   = 25_000_000,

    // Grupo 1 (blink e q[8:6])
    parameter int unsigned ON_MS      = 500,
    parameter int unsigned OFF_MS     = 500,

    // Grupo 2 (q[5:3])
    parameter int unsigned G2_ON_MS   = 250,
    parameter int unsigned G2_OFF_MS  = 250,

    // Grupo 3 (q[2:0])
    parameter int unsigned G3_ON_MS   = 1000,
    parameter int unsigned G3_OFF_MS  = 1000
)(
    input  logic clk,
    output logic led,
    output logic [8:0] q
);
    // ---------- util: ms -> ticks ----------
    localparam int unsigned TICKS_PER_MS = (F_CLK_HZ/1000);

    // ---------- Grupo 1: blink + contador q[8:6] ----------
    localparam int unsigned G1_ON_TKS   = TICKS_PER_MS * ON_MS;
    localparam int unsigned G1_OFF_TKS  = TICKS_PER_MS * OFF_MS;
    localparam int unsigned G1_PERIOD   = G1_ON_TKS + G1_OFF_TKS;
    localparam int unsigned G1_W        = (G1_PERIOD > 1) ? $clog2(G1_PERIOD) : 1;

    logic [G1_W-1:0] g1_cnt = '0;
    always_ff @(posedge clk) begin
        g1_cnt <= (g1_cnt == G1_PERIOD-1) ? '0 : (g1_cnt + 1'b1);
    end

    wire g1_wave = (g1_cnt < G1_ON_TKS);
    assign led   = g1_wave;

    // “tick” de avanço do contador 3b do grupo 1: 1 ciclo quando o período termina
    wire g1_tick = (g1_cnt == G1_PERIOD-1);

    logic [2:0] g1_q = '0;         // contador de 3 bits
    always_ff @(posedge clk) begin
        if (g1_tick) g1_q <= g1_q + 3'd1;  // wrap automático em 3 bits
    end

    // ---------- Grupo 2: contador q[5:3] ----------
    localparam int unsigned G2_ON_TKS  = TICKS_PER_MS * G2_ON_MS;
    localparam int unsigned G2_OFF_TKS = TICKS_PER_MS * G2_OFF_MS;
    localparam int unsigned G2_PERIOD  = G2_ON_TKS + G2_OFF_TKS;
    localparam int unsigned G2_W       = (G2_PERIOD > 1) ? $clog2(G2_PERIOD) : 1;

    logic [G2_W-1:0] g2_cnt = '0;
    always_ff @(posedge clk) begin
        g2_cnt <= (g2_cnt == G2_PERIOD-1) ? '0 : (g2_cnt + 1'b1);
    end

    wire       g2_tick = (g2_cnt == G2_PERIOD-1);
    logic [2:0] g2_q   = '0;
    always_ff @(posedge clk) begin
        if (g2_tick) g2_q <= g2_q + 3'd1;
    end

    // ---------- Grupo 3: contador q[2:0] ----------
    localparam int unsigned G3_ON_TKS  = TICKS_PER_MS * G3_ON_MS;
    localparam int unsigned G3_OFF_TKS = TICKS_PER_MS * G3_OFF_MS;
    localparam int unsigned G3_PERIOD  = G3_ON_TKS + G3_OFF_TKS;
    localparam int unsigned G3_W       = (G3_PERIOD > 1) ? $clog2(G3_PERIOD) : 1;

    logic [G3_W-1:0] g3_cnt = '0;
    always_ff @(posedge clk) begin
        g3_cnt <= (g3_cnt == G3_PERIOD-1) ? '0 : (g3_cnt + 1'b1);
    end

    wire       g3_tick = (g3_cnt == G3_PERIOD-1);
    logic [2:0] g3_q   = '0;
    always_ff @(posedge clk) begin
        if (g3_tick) g3_q <= g3_q + 3'd1;
    end

    // ---------- Mapeamento para a saída q ----------
    // Convenção: bit mais significativo à esquerda
    assign q[8:6] = g1_q;  // MSB..LSB do contador do grupo 1
    assign q[5:3] = g2_q;  // grupo 2
    assign q[2:0] = g3_q;  // grupo 3
endmodule
