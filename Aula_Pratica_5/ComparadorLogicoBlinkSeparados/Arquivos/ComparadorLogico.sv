// ComparadorLogico.sv (limpo)
module ComparadorLogico #(
    parameter bit ACTIVE_LOW = 0
)(
    // Palavra Superior
    input  logic in_a,  // S2
    input  logic in_b,  // S1
    input  logic in_c,  // S0
    // Palavra Inferior
    input  logic in_d,  // I2
    input  logic in_e,  // I1
    input  logic in_f,  // I0
    // Saída
    output logic [11:0] y
);
    // (mesma lógica de antes…)
    wire S2 = ACTIVE_LOW ? ~in_a : in_a;
    wire S1 = ACTIVE_LOW ? ~in_b : in_b;
    wire S0 = ACTIVE_LOW ? ~in_c : in_c;

    wire I2 = ACTIVE_LOW ? ~in_d : in_d;
    wire I1 = ACTIVE_LOW ? ~in_e : in_e;
    wire I0 = ACTIVE_LOW ? ~in_f : in_f;

    wire [2:0] Superior = {S2,S1,S0};
    wire [2:0] Inferior  = {I2,I1,I0};

    assign y[0]  = (~S0 &  I0);
    assign y[1]  = (~S1 &  I1);
    assign y[2]  = (~S2 &  I2);
    assign y[3]  = ~(S0 ^ I0);
    assign y[4]  = ~(S1 ^ I1);
    assign y[5]  = ~(S2 ^ I2);
    assign y[6]  = ( S0 & ~I0);
    assign y[7]  = ( S1 & ~I1);
    assign y[8]  = ( S2 & ~I2);
    assign y[9]  = (Superior <  Inferior);
    assign y[10] = (Superior == Inferior);
    assign y[11] = (Superior >  Inferior);
endmodule
