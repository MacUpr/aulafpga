`include "comb_with_wire.sv"
`include "mux_reg_with_legacy.sv"
`include "dff_with_logic.sv"

module top(
  input  logic a, b, sel,
  input  logic clk, rst_n,
  output logic y_comb,
  output logic y_mux,
  output logic q_dff
);
  comb_with_wire      u_comb(.a(a), .b(b), .y(y_comb));
  mux_with_reg_legacy u_mux (.a(a), .b(b), .sel(sel), .y(y_mux));
  dff_with_logic      u_dff (.clk(clk), .rst_n(rst_n), .d(a ^ b), .q(q_dff));
endmodule
