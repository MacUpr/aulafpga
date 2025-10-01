// ====================================================================
// top.sv — Comparador: Palavra Superior (S2..S0) vs Palavra Inferior (I2..I0)
// Pinos:
//   Superior = {in_a (S2, MSB), in_b (S1), in_c (S0, LSB)}
//   Inferior = {in_d (I2, MSB), in_e (I1), in_f (I0, LSB)}
//
// Saídas (y[0]..y[11]) conforme especificado:
//   y[0]  = (S0 < I0)          // bit LSB: Superior[0] < Inferior[0]
//   y[1]  = (S1 < I1)
//   y[2]  = (S2 < I2)
//   y[3]  = (S0 == I0)
//   y[4]  = (S1 == I1)
//   y[5]  = (S2 == I2)
//   y[6]  = (S0 > I0)
//   y[7]  = (S1 > I1)
//   y[8]  = (S2 > I2)
//   y[9]  = (Superior < Inferior)   // comparação de palavra (3 bits, sem sinal)
//   y[10] = (Superior == Inferior)
//   y[11] = (Superior > Inferior)
// ====================================================================
module top #(
    // Se seus sinais de entrada vierem ativos-baixo (ex.: pull-up com botão),
    // coloque ACTIVE_LOW=1 para normalizar. Caso use níveis 0/1 diretos (toggle),
    // deixe ACTIVE_LOW=0 (padrão).
    parameter bit ACTIVE_LOW = 0
) (
    input  logic clk,   // não usado; mantido para compatibilidade com o .lpf

    // Palavra Superior (conector superior)
    input  logic in_a,  // S2 (MSB)
    input  logic in_b,  // S1
    input  logic in_c,  // S0 (LSB)

    // Palavra Inferior (conector inferior)
    input  logic in_d,  // I2 (MSB)
    input  logic in_e,  // I1
    input  logic in_f,  // I0 (LSB)

    output logic led,         // mantido; aqui desligado
    output logic [11:0] y
);

    // -----------------------------
    // Normalização (se ACTIVE_LOW)
    // -----------------------------
    wire S2 = ACTIVE_LOW ? ~in_a : in_a;
    wire S1 = ACTIVE_LOW ? ~in_b : in_b;
    wire S0 = ACTIVE_LOW ? ~in_c : in_c;

    wire I2 = ACTIVE_LOW ? ~in_d : in_d;
    wire I1 = ACTIVE_LOW ? ~in_e : in_e;
    wire I0 = ACTIVE_LOW ? ~in_f : in_f;

    wire [2:0] Superior = {S2, S1, S0};   // MSB..LSB
    wire [2:0] Inferior = {I2, I1, I0};   // MSB..LSB

    // ---------------------------------
    // Comparações por bit (combinacional)
    // ---------------------------------
    // Para 1-bit:  (S < I) = (~S & I), (S == I) = ~(S ^ I), (S > I) = (S & ~I)
    assign y[0] = (~S0 &  I0);     // S0 < I0
    assign y[1] = (~S1 &  I1);     // S1 < I1
    assign y[2] = (~S2 &  I2);     // S2 < I2

    assign y[3] = ~(S0 ^ I0);      // S0 == I0
    assign y[4] = ~(S1 ^ I1);      // S1 == I1
    assign y[5] = ~(S2 ^ I2);      // S2 == I2

    assign y[6] = ( S0 & ~I0);     // S0 > I0
    assign y[7] = ( S1 & ~I1);     // S1 > I1
    assign y[8] = ( S2 & ~I2);     // S2 > I2

    // ---------------------------------
    // Comparação de palavra (3 bits, unsigned)
    // ---------------------------------
    assign y[9]  = (Superior <  Inferior);
    assign y[10] = (Superior == Inferior);
    assign y[11] = (Superior >  Inferior);

    // LED mantido desligado (compatibilidade com .lpf)
    assign led = 1'b0;

endmodule
