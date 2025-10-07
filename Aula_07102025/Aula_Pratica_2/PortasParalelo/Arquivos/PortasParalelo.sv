// ====================================================================
// top.sv — Blink ÚNICO + 2x conjuntos de I/O (duas BitDogLabs)
//          - 1 contador de blink (único)
//          - 1 saída 'led' (escalares)
//          - Dois trios de entradas (in_*[1:0]) e lógicas por placa
// ====================================================================

module top #(
    // Clock de entrada, em Hz
    parameter int unsigned F_CLK_HZ = 25_000_000,

    // Janelas de tempo do LED em milissegundos
    parameter int unsigned ON_MS  = 500,   // tempo em nível alto
    parameter int unsigned OFF_MS = 500    // tempo em nível baixo
) (
    input  logic clk,          // clock de sistema (único)

    // -------- Entradas dos botões (duplicadas: [0] = Placa A, [1] = Placa B)
    input  logic [1:0] in_a,
    input  logic [1:0] in_b,
    input  logic [1:0] in_c,

    // -------- Saída de blink (ÚNICA)
    output logic       led,

    // Combinações lógicas independentes por placa
    output logic [1:0] y_or,
    output logic [1:0] y_and,
    output logic [1:0] y_nor,
    output logic [1:0] y_nand,
    output logic [1:0] y_xor,
    output logic [1:0] y_xnor
);

    // ----------------------------------------------------------------
    // Blink compartilhado (um único contador e uma única saída)
    // ----------------------------------------------------------------
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

    wire blink_sig = (cnt < ON_TICKS);
    assign led = blink_sig;  // única saída de blink

    // ----------------------------------------------------------------
    // Lógica combinacional por placa (usa entradas independentes)
    // ----------------------------------------------------------------
    for (genvar i = 0; i < 2; i++) begin : gen_logic_per_board
        assign y_or  [i] =  in_a[i] |  in_b[i] |  in_c[i];
        assign y_and [i] =  in_a[i] &  in_b[i] &  in_c[i];
        assign y_nor [i] = ~(in_a[i] |  in_b[i] |  in_c[i]);
        assign y_nand[i] = ~(in_a[i] &  in_b[i] &  in_c[i]);
        assign y_xor [i] =  in_a[i] ^  in_b[i] ^  in_c[i];    // paridade ímpar
        assign y_xnor[i] = ~(in_a[i] ^  in_b[i] ^  in_c[i]);  // paridade par
    end

endmodule
