// ====================================================================
// top.sv — Blink + q[8:0] na MESMA frequência (e fase) do led
// ====================================================================
module top #(
    parameter int unsigned F_CLK_HZ = 25_000_000,
    parameter int unsigned ON_MS    = 500,
    parameter int unsigned OFF_MS   = 500
)(
    input  logic clk,
    output logic led,
    output logic [8:0] q
);
    // ---------- Blink ----------
    localparam int unsigned ON_TICKS   = (F_CLK_HZ/1000) * ON_MS;
    localparam int unsigned OFF_TICKS  = (F_CLK_HZ/1000) * OFF_MS;
    localparam int unsigned PERIOD_TKS = ON_TICKS + OFF_TICKS;
    localparam int unsigned CNT_W      = (PERIOD_TKS > 1) ? $clog2(PERIOD_TKS) : 1;

    logic [CNT_W-1:0] cnt = '0;

    always_ff @(posedge clk) begin
        cnt <= (cnt == PERIOD_TKS-1) ? '0 : (cnt + 1'b1);
    end

    // led alto por ON_TICKS e baixo por OFF_TICKS
    assign led = (cnt < ON_TICKS);

    // ---------- q muda na mesma frequência do led ----------
    // Opção combinacional (mesma forma de onda):
    assign q = {9{led}};

    // Se preferir registrado (muda só na borda do clock), troque por:
    // always_ff @(posedge clk) q <= {9{led}};

endmodule
