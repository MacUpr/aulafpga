// ====================================================================
// top_ex1 — Blink + O1..O5 seguem I1 (com suporte a LED ativo-baixo)
// ====================================================================
module top_ex1 #(
    parameter int unsigned F_CLK_HZ   = 25_000_000,
    parameter int unsigned ON_MS      = 500,
    parameter int unsigned OFF_MS     = 500,
    parameter bit          LED_ACTIVE_LOW = 0  // 0=ativo-alto, 1=ativo-baixo
)(
    input  logic clk,               // clock (ex.: 25 MHz)
    input  logic I1,                // entrada (ativo-alto)
    output logic led,               // LED blink (com inversão opcional)
    output logic O1, O2, O3, O4, O5 // saídas (ativo-alto)
);
    // -------- Blink --------
    localparam int unsigned ON_T   = (F_CLK_HZ/1000) * ON_MS;
    localparam int unsigned OFF_T  = (F_CLK_HZ/1000) * OFF_MS;
    localparam int unsigned PERIOD = ON_T + OFF_T;
    localparam int unsigned W      = (PERIOD > 1) ? $clog2(PERIOD) : 1;

    logic [W-1:0] cnt = '0;
    always_ff @(posedge clk)
        cnt <= (cnt == PERIOD-1) ? '0 : (cnt + 1'b1);

    wire led_on = (cnt < ON_T);
    assign led  = LED_ACTIVE_LOW ? ~led_on : led_on;

    // -------- O1..O5 seguem I1 --------
    assign {O5, O4, O3, O2, O1} = {5{I1}};
endmodule
