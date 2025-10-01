// ====================================================================
// top.sv — Blink + 3 botões controlando 6 saídas (versão simples)
// Regras: q começa em 6'b111111; A=0 -> q0,q1=0; B=0 -> q2,q3=0; C=0 -> q4,q5=0
// Se ACTIVE_LOW_BTN=1: botão pressionado = 0. Se =0: botão pressionado = 1.
// ====================================================================

module top #(
    parameter int unsigned F_CLK_HZ       = 25_000_000,
    parameter int unsigned ON_MS          = 500,
    parameter int unsigned OFF_MS         = 500,
    parameter bit          ACTIVE_LOW_BTN = 1  // 1=ativo baixo, 0=ativo alto
)(
    input  logic clk,
    input  logic btn_a, btn_b, btn_c,   // três botões
    output logic led,                   // blink
    output logic [5:0] q                // seis saídas
);
    // ---------- Blink ----------
    localparam int unsigned ON_TICKS   = (F_CLK_HZ/1000) * ON_MS;
    localparam int unsigned OFF_TICKS  = (F_CLK_HZ/1000) * OFF_MS;
    localparam int unsigned PERIOD_TKS = ON_TICKS + OFF_TICKS;
    localparam int unsigned CNT_W      = (PERIOD_TKS > 1) ? $clog2(PERIOD_TKS) : 1;

    logic [CNT_W-1:0] cnt = '0;
    always_ff @(posedge clk)
        cnt <= (cnt == PERIOD_TKS-1) ? '0 : cnt + 1'b1;

    assign led = (cnt < ON_TICKS);

    // ---------- Mapeamento simples dos botões para as saídas ----------
    // level_x é o nível que queremos nas saídas (1 normal, 0 quando botão "ativo")
    wire a_level = ACTIVE_LOW_BTN ? btn_a : ~btn_a;
    wire b_level = ACTIVE_LOW_BTN ? btn_b : ~btn_b;
    wire c_level = ACTIVE_LOW_BTN ? btn_c : ~btn_c;

    // q[1:0] = A, q[3:2] = B, q[5:4] = C
    assign q = { {2{c_level}}, {2{b_level}}, {2{a_level}} };

endmodule
