`include "register_bank.sv"

module top #(
  parameter int WIDTH = 16,
  parameter int DEPTH = 8
)(
  input  logic                     clk,
  input  logic                     rst_n,

  // Interface de escrita
  input  logic                     we,
  input  logic [$clog2(DEPTH)-1:0] waddr,
  input  logic [WIDTH-1:0]         wdata,

  // Interfaces de leitura (duas portas)
  input  logic [$clog2(DEPTH)-1:0] raddr_a,
  output logic [WIDTH-1:0]         rdata_a,
  input  logic [$clog2(DEPTH)-1:0] raddr_b,
  output logic [WIDTH-1:0]         rdata_b
);

  register_bank #(.WIDTH(WIDTH), .DEPTH(DEPTH)) u_rf (
    .clk     (clk),
    .rst_n   (rst_n),
    .we      (we),
    .waddr   (waddr),
    .wdata   (wdata),
    .raddr_a (raddr_a),
    .rdata_a (rdata_a),
    .raddr_b (raddr_b),
    .rdata_b (rdata_b)
  );

endmodule

