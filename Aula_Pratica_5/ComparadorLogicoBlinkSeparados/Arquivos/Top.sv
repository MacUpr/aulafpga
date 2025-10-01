// top.sv — y[11:0] = ComparadorLogico, led = Blink
module top #(
    parameter bit          ACTIVE_LOW      = 0,          // entradas do comparador
    parameter bit          ACTIVE_LOW_LED  = 0,          // 1 se LED físico for ativo-baixo
    parameter int unsigned F_CLK_HZ        = 25_000_000, // Blink
    parameter int unsigned ON_MS           = 250,
    parameter int unsigned OFF_MS          = 250
)(
    input  logic clk,
    input  logic in_a, in_b, in_c, in_d, in_e, in_f,
    output logic led,
    output logic [11:0] y
);
    // Comparador → y (sem clk/led)
    ComparadorLogico #(.ACTIVE_LOW(ACTIVE_LOW)) u_comp (
        .in_a(in_a), .in_b(in_b), .in_c(in_c),
        .in_d(in_d), .in_e(in_e), .in_f(in_f),
        .y   (y)
    );

    // Blink → led
    logic led_raw;
    Blink #(
        .F_CLK_HZ(F_CLK_HZ), .ON_MS(ON_MS), .OFF_MS(OFF_MS)
    ) u_blink (
        .clk (clk),
        .led (led_raw)
    );

    // Ajuste de polaridade do LED físico
    assign led = (ACTIVE_LOW_LED) ? ~led_raw : led_raw;
endmodule
