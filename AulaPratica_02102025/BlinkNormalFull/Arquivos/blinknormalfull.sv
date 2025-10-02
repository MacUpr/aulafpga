// ====================================================================
// top.sv — Blink + três contadores de 3 bits (q[8:6], q[5:3], q[2:0])
// - q_copy[5:0] espelha q[5:0]
// - Entradas ATIVAS-BAIXAS:
//     * reset  (0 = reset;  1 = normal) → zera q e reinicia fases
//     * preset (0 = ativo;  1 = normal) → força q = 3'b111 em cada grupo
//     * pause  (0 = pausa;  1 = conta)  → congela SOMENTE os contadores q
// - O LED continua piscando independentemente de pause/preset.
// ====================================================================
module top #(
    parameter int unsigned F_CLK_HZ   = 25_000_000,

    // Grupo 1 (blink e q[8:6])
    parameter int unsigned ON_MS      = 500,
    parameter int unsigned OFF_MS     = 500,

    // Grupo 2 (q[5:3])
    parameter int unsigned G2_ON_MS   = 200,
    parameter int unsigned G2_OFF_MS  = 800,

    // Grupo 3 (q[2:0])
    parameter int unsigned G3_ON_MS   = 100,
    parameter int unsigned G3_OFF_MS  = 100
)(
    input  logic clk,
    input  logic reset,    // ativo-baixo
    input  logic preset,   // ativo-baixo
    input  logic pause,    // ativo-baixo

    output logic led,
    output logic [8:0] q,
    output logic [5:0] q_copy
);
    // --------- decodificação de níveis ativos-baixos ---------
    wire rst  = ~reset;   // 1 quando reset está ativo
    wire pset = ~preset;  // 1 quando preset está ativo
    wire hold = ~pause;   // 1 quando pausar contadores de saída

    // ---------- util: ms -> ticks ----------
    localparam int unsigned TICKS_PER_MS = (F_CLK_HZ/1000);

    // ---------- Grupo 1: blink + contador q[8:6] ----------
    localparam int unsigned G1_ON_TKS   = TICKS_PER_MS * ON_MS;
    localparam int unsigned G1_OFF_TKS  = TICKS_PER_MS * OFF_MS;
    localparam int unsigned G1_PERIOD   = G1_ON_TKS + G1_OFF_TKS;
    localparam int unsigned G1_W        = (G1_PERIOD > 1) ? $clog2(G1_PERIOD) : 1;

    logic [G1_W-1:0] g1_cnt = '0;
    always_ff @(posedge clk) begin
        if (rst) g1_cnt <= '0;
        else     g1_cnt <= (g1_cnt == G1_PERIOD-1) ? '0 : (g1_cnt + 1'b1);
    end

    wire g1_wave = (g1_cnt < G1_ON_TKS);
    assign led   = g1_wave;

    wire g1_tick = (g1_cnt == G1_PERIOD-1);

    logic [2:0] g1_q = '0;  // q[8:6]
    always_ff @(posedge clk) begin
        if (rst)                     g1_q <= 3'd0;
        else if (pset)               g1_q <= 3'b111;   // preset → força 111
        else if (!hold && g1_tick)   g1_q <= g1_q + 3'd1;
    end

    // ---------- Grupo 2: q[5:3] ----------
    localparam int unsigned G2_ON_TKS  = TICKS_PER_MS * G2_ON_MS;
    localparam int unsigned G2_OFF_TKS = TICKS_PER_MS * G2_OFF_MS;
    localparam int unsigned G2_PERIOD  = G2_ON_TKS + G2_OFF_TKS;
    localparam int unsigned G2_W       = (G2_PERIOD > 1) ? $clog2(G2_PERIOD) : 1;

    logic [G2_W-1:0] g2_cnt = '0;
    always_ff @(posedge clk) begin
        if (rst) g2_cnt <= '0;
        else     g2_cnt <= (g2_cnt == G2_PERIOD-1) ? '0 : (g2_cnt + 1'b1);
    end

    wire       g2_tick = (g2_cnt == G2_PERIOD-1);
    logic [2:0] g2_q   = '0;
    always_ff @(posedge clk) begin
        if (rst)                     g2_q <= 3'd0;
        else if (pset)               g2_q <= 3'b111;   // preset → força 111
        else if (!hold && g2_tick)   g2_q <= g2_q + 3'd1;
    end

    // ---------- Grupo 3: q[2:0] ----------
    localparam int unsigned G3_ON_TKS  = TICKS_PER_MS * G3_ON_MS;
    localparam int unsigned G3_OFF_TKS = TICKS_PER_MS * G3_OFF_MS;
    localparam int unsigned G3_PERIOD  = G3_ON_TKS + G3_OFF_TKS;
    localparam int unsigned G3_W       = (G3_PERIOD > 1) ? $clog2(G3_PERIOD) : 1;

    logic [G3_W-1:0] g3_cnt = '0;
    always_ff @(posedge clk) begin
        if (rst) g3_cnt <= '0;
        else     g3_cnt <= (g3_cnt == G3_PERIOD-1) ? '0 : (g3_cnt + 1'b1);
    end

    wire       g3_tick = (g3_cnt == G3_PERIOD-1);
    logic [2:0] g3_q   = '0;
    always_ff @(posedge clk) begin
        if (rst)                     g3_q <= 3'd0;
        else if (pset)               g3_q <= 3'b111;   // preset → força 111
        else if (!hold && g3_tick)   g3_q <= g3_q + 3'd1;
    end

    // ---------- Mapeamento das saídas ----------
    assign q[8:6] = g1_q;
    assign q[5:3] = g2_q;
    assign q[2:0] = g3_q;

    assign q_copy = q[5:0];  // espelho q[0]..q[5]
endmodule
