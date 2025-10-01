module top_ex1 (
    input  wire clk,              // P3 - clock 25 MHz
    input  wire I1, I2, I3, I4, I5,   // Entradas (D1, C1, C2, E3, B4) - ativo alto
    output logic O1, O2, O3, O4, O5   // Saídas (E2, D2, B1, A3, C3) - ativo alto
);

    // Todas as saídas seguem o estado de I1
    always_comb begin
        if (I1) begin
            O1 = 1;
            O2 = 1;
            O3 = 1;
            O4 = 1;
            O5 = 1;
        end else begin
            O1 = 0;
            O2 = 0;
            O3 = 0;
            O4 = 0;
            O5 = 0;
        end
    end

endmodule
