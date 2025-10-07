// ====================================================================
// top.sv — Blink único + 6 entradas + 12 saídas (cada saída usa 4 entradas)
// Sintetizável (Lattice Diamond / Synplify)
// ====================================================================

module top #(
    // Clock de entrada, em Hz
    parameter int unsigned F_CLK_HZ = 25_000_000,

    // Janelas do blink em milissegundos
    parameter int unsigned ON_MS  = 500,
    parameter int unsigned OFF_MS = 500
) (
    input  logic clk,       // clock de sistema

    // 6 entradas distintas
    input  logic in_a,
    input  logic in_b,
    input  logic in_c,
    input  logic in_d,
    input  logic in_e,
    input  logic in_f,

    // Blink único
    output logic led,

    // 12 saídas distintas (cada uma combina 4 das 6 entradas)
    output logic [11:0] y
);

    // ----------------------------------------------------------------
    // Blink (um único contador)
    // ----------------------------------------------------------------
    localparam int unsigned ON_TICKS   = (F_CLK_HZ / 1000) * ON_MS;
    localparam int unsigned OFF_TICKS  = (F_CLK_HZ / 1000) * OFF_MS;
    localparam int unsigned PERIOD_TKS = ON_TICKS + OFF_TICKS;
    localparam int unsigned CNT_W      = (PERIOD_TKS > 1) ? $clog2(PERIOD_TKS) : 1;

    logic [CNT_W-1:0] cnt = '0;

    always_ff @(posedge clk) begin
        if (cnt == PERIOD_TKS - 1)
            cnt <= '0;
        else
            cnt <= cnt + 1'b1;
    end

    assign led = (cnt < ON_TICKS);

    // ----------------------------------------------------------------
    // Atalhos legíveis
    // ----------------------------------------------------------------
    wire a = in_a, b = in_b, c = in_c, d = in_d, e = in_e, f = in_f;

    // ----------------------------------------------------------------
    // 12 funções lógicas (cada uma usa 4 variáveis das 6)
    // — Troque as expressões à vontade; todas são sintetizáveis.
    // — As combinações foram escolhidas para “espalhar” o uso de a..f.
    // ----------------------------------------------------------------

    // y[0] — usa a,b,c,d  |  (A+B)·(C+D)
    assign y[0]  = (a | b) & (c | d);

    // y[1] — usa a,b,c,d  |  (A·B) + (C·D)
    assign y[1]  = (a & b) | (c & d);

    // y[2] — usa a,b,c,d  |  (A ⊕ B) ⊕ (C · D)
    assign y[2]  = (a ^ b) ^ (c & d);

    // y[3] — usa a,b,c,d  |  (~A·C) + (B·~D)
    assign y[3]  = (~a & c) | (b & ~d);

    // y[4] — usa a,b,e,f  |  (A·~B) + (E·~F)
    assign y[4]  = (a & ~b) | (e & ~f);

    // y[5] — usa a,c,e,f  |  (A ⊕ C) · (E + F)
    assign y[5]  = (a ^ c) & (e | f);

    // y[6] — usa b,c,e,f  |  (B ⊙ C) ⊕ (E · F)   (⊙ = XNOR)
    assign y[6]  = (b ~^ c) ^ (e & f);

    // y[7] — usa b,d,e,f  |  (~B·E) + (D·~F)
    assign y[7]  = (~b & e) | (d & ~f);

    // y[8] — usa c,d,e,f  |  (C + D) · (~E + F)
    assign y[8]  = (c | d) & ((~e) | f);

    // y[9] — usa a,d,e,f  |  (A·E) + (D·(E ⊕ F))
    assign y[9]  = (a & e) | (d & (e ^ f));

    // y[10] — usa a,c,d,f |  A ⊕ C ⊕ D ⊕ F  (paridade de 4)
    assign y[10] = a ^ c ^ d ^ f;

    // y[11] — usa b,c,d,e |  (B·~C) + (~D·E)
    assign y[11] = (b & ~c) | (~d & e);

endmodule
