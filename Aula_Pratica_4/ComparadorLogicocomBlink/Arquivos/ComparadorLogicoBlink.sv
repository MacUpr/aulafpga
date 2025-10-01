// ====================================================================
// top.sv — Comparador 3b (S2..S0 vs I2..I0) + Blink parametrizável
// Sintetizável em Lattice Diamond / Synplify (SystemVerilog)
// ====================================================================

module top #(
    // Comparador
    parameter bit          ACTIVE_LOW      = 0,           // 1 se entradas vêm ativas-baixas (pull-up)
    // Blink
    parameter int unsigned F_CLK_HZ        = 25_000_000,  // Hz
    parameter int unsigned ON_MS           = 250,         // ms nível alto (antes da inversão opcional)
    parameter int unsigned OFF_MS          = 250,         // ms nível baixo (antes da inversão opcional)
    parameter bit          ACTIVE_LOW_LED  = 0            // 1 se LED físico for ativo-baixo
)(
    input  logic clk,        // clock de sistema (P3)
    // Palavra Superior (conector superior)
    input  logic in_a,       // S2 (MSB)
    input  logic in_b,       // S1
    input  logic in_c,       // S0 (LSB)
    // Palavra Inferior (conector inferior)
    input  logic in_d,       // I2 (MSB)
    input  logic in_e,       // I1
    input  logic in_f,       // I0 (LSB)

    output logic led,        // LED on-board (L2)
    output logic [11:0] y    // saídas do comparador
);

    // ================================================================
    // 1) COMPARADOR 3 bits — exatamente como o seu código que funciona
    // ================================================================
    // Normalização (se ACTIVE_LOW)
    wire S2 = ACTIVE_LOW ? ~in_a : in_a;
    wire S1 = ACTIVE_LOW ? ~in_b : in_b;
    wire S0 = ACTIVE_LOW ? ~in_c : in_c;

    wire I2 = ACTIVE_LOW ? ~in_d : in_d;
    wire I1 = ACTIVE_LOW ? ~in_e : in_e;
    wire I0 = ACTIVE_LOW ? ~in_f : in_f;

    wire [2:0] Superior = {S2, S1, S0};   // MSB..LSB
    wire [2:0] Inferior = {I2, I1, I0};   // MSB..LSB

    // Comparações por bit (combinacional)
    // Para 1-bit: (S<I)=(~S&I), (S==I)=~(S^I), (S>I)=(S&~I)
    assign y[0] = (~S0 &  I0);     // S0 < I0
    assign y[1] = (~S1 &  I1);     // S1 < I1
    assign y[2] = (~S2 &  I2);     // S2 < I2

    assign y[3] = ~(S0 ^ I0);      // S0 == I0
    assign y[4] = ~(S1 ^ I1);      // S1 == I1
    assign y[5] = ~(S2 ^ I2);      // S2 == I2

    assign y[6] = ( S0 & ~I0);     // S0 > I0
    assign y[7] = ( S1 & ~I1);     // S1 > I1
    assign y[8] = ( S2 & ~I2);     // S2 > I2

    // Comparação de palavra (3 bits, unsigned)
    assign y[9]  = (Superior <  Inferior);
    assign y[10] = (Superior == Inferior);
    assign y[11] = (Superior >  Inferior);

    // =========================================
    // 2) BLINK parametrizável — igual ao seu
    // =========================================
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

    wire led_raw = (cnt < ON_TICKS);               // forma de onda básica
    assign led   = ACTIVE_LOW_LED ? ~led_raw : led_raw; // ajuste de polaridade física

endmodule
