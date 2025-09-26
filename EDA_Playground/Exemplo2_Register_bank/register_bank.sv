module register_bank #(
  parameter int WIDTH = 16,           // Largura de cada registrador (bits)
  parameter int DEPTH = 8             // Quantidade de registradores
)(
  input  logic                     clk,
  input  logic                     rst_n,

  // Porta de escrita
  input  logic                     we,             // write enable
  input  logic [$clog2(DEPTH)-1:0] waddr,          // endereço de escrita
  input  logic [WIDTH-1:0]         wdata,          // dado de escrita

  // Duas portas de leitura combinacional
  input  logic [$clog2(DEPTH)-1:0] raddr_a,        // endereço leitura A
  output logic [WIDTH-1:0]         rdata_a,        // dado leitura A
  input  logic [$clog2(DEPTH)-1:0] raddr_b,        // endereço leitura B
  output logic [WIDTH-1:0]         rdata_b         // dado leitura B
);

  // Array fixo: DEPTH elementos, cada um com WIDTH bits
  logic [WIDTH-1:0] mem [0:DEPTH-1];

  // Escrita síncrona + reset assíncrono
  always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
      for (int i = 0; i < DEPTH; i++) begin
        mem[i] <= '0;
      end
    end else if (we) begin
      mem[waddr] <= wdata;
    end
  end

  // Leituras combinacionais (assíncronas)
  assign rdata_a = mem[raddr_a];
  assign rdata_b = mem[raddr_b];

endmodule
