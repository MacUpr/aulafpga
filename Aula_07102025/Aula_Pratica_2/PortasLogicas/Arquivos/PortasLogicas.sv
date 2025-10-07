// ====================================================================
// top.sv  — Blink parametrizável (ON/OFF em ms)
//           + combinações lógicas de 3 entradas (OR/AND/NOR/NAND/XOR/XNOR)
// Sintetizável em Lattice Diamond / Synplify (SystemVerilog)
// ====================================================================

module top #(
    // Clock de entrada, em Hz
    parameter int unsigned F_CLK_HZ = 25_000_000,

    // Janelas de tempo do LED em milissegundos
    parameter int unsigned ON_MS  = 500,   // tempo em nível alto
    parameter int unsigned OFF_MS = 500    // tempo em nível baixo
) (
    input  logic clk,        // clock de sistema

    // Entradas para lógica combinacional
    input  logic in_a,
    input  logic in_b,
    input  logic in_c,

    // Saídas
    output logic led,        // saída do "blink"
    output logic y_or,
    output logic y_and,
    output logic y_nor,
    output logic y_nand,
    output logic y_xor,
    output logic y_xnor
);

    // ----------------------------------------------------------------
    // Conversão ms -> ticks
    // (Para 25 MHz e 500 ms: 12_500_000, cabe em 32 bits.)
    // ----------------------------------------------------------------
    localparam int unsigned ON_TICKS   = (F_CLK_HZ / 1000) * ON_MS;
    localparam int unsigned OFF_TICKS  = (F_CLK_HZ / 1000) * OFF_MS;
    localparam int unsigned PERIOD_TKS = ON_TICKS + OFF_TICKS;

    // Largura mínima do contador (>= 1)
    localparam int unsigned CNT_W = (PERIOD_TKS > 1) ? $clog2(PERIOD_TKS) : 1;

    // Contador livre de reset (inicialização útil na simulação)
    logic [CNT_W-1:0] cnt = '0;

    // ----------------------------------------------------------------
    // Contador de período
    // ----------------------------------------------------------------
    always_ff @(posedge clk) begin
        if (cnt == PERIOD_TKS - 1)
            cnt <= '0;
        else
            cnt <= cnt + 1'b1;
    end

    // Região "ON" do período
    wire blink_on = (cnt < ON_TICKS);

    // Saída do blink
    assign led = blink_on;

    // ----------------------------------------------------------------
    // Combinações lógicas dos três sinais de entrada
    // ----------------------------------------------------------------
    assign y_or   =  in_a |  in_b |  in_c;          // OR
    assign y_and  =  in_a &  in_b &  in_c;          // AND
    assign y_nor  = ~(in_a |  in_b |  in_c);        // NOR
    assign y_nand = ~(in_a &  in_b &  in_c);        // NAND
    assign y_xor  =  in_a ^  in_b ^  in_c;          // XOR (paridade ímpar)
    assign y_xnor = ~(in_a ^  in_b ^  in_c);        // XNOR (NXOR, paridade par)

endmodule
