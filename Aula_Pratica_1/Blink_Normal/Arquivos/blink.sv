// ====================================================================
// top.sv  — Blink parametrizável (ON/OFF em ms)
// Sintetizável em Lattice Diamond / Synplify (SystemVerilog)
// ====================================================================

module top #(
    // Clock de entrada, em Hz
    parameter int unsigned F_CLK_HZ = 25_000_000,

    // Janelas de tempo do LED em milissegundos
    parameter int unsigned ON_MS  = 250,   // tempo em nível alto
    parameter int unsigned OFF_MS = 250    // tempo em nível baixo
) (
    input  logic clk,   // clock de sistema
    output logic led    // saída do "blink" (pino ou LED on-board)
);

    // ----------------------------------------------------------------
    // Conversão ms -> ticks
    // ----------------------------------------------------------------
    localparam int unsigned ON_TICKS   = (F_CLK_HZ / 1000) * ON_MS;
    localparam int unsigned OFF_TICKS  = (F_CLK_HZ / 1000) * OFF_MS;
    localparam int unsigned PERIOD_TKS = ON_TICKS + OFF_TICKS;

    // Largura mínima do contador (>= 1)
    localparam int unsigned CNT_W = (PERIOD_TKS > 1) ? $clog2(PERIOD_TKS) : 1;

    // Contador livre de reset (sem necessidade de reset externo)
    logic [CNT_W-1:0] cnt = '0;  // inicialização ajuda na simulação

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
    assign led = (cnt < ON_TICKS);

endmodule
