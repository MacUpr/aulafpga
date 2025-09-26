module tb;

  localparam int WIDTH = 16;
  localparam int DEPTH = 8;
  localparam int AW    = $clog2(DEPTH);

  // Sinais
  logic                   clk, rst_n;
  logic                   we;
  logic [AW-1:0]          waddr;
  logic [WIDTH-1:0]       wdata;
  logic [AW-1:0]          raddr_a, raddr_b;
  logic [WIDTH-1:0]       rdata_a, rdata_b;

  // DUT
  top #(.WIDTH(WIDTH), .DEPTH(DEPTH)) dut (
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

  // Clock 100 MHz (10 ns)
  initial clk = 1'b0;
  always  #5 clk = ~clk;

  // Waveform (EPWave/GTKWave)
  initial begin
    $dumpfile("waveform.vcd");
    $dumpvars(0, tb);
  end

  // Tarefas auxiliares
  task automatic write_reg(input logic [AW-1:0] addr, input logic [WIDTH-1:0] data);
    begin
      @(negedge clk);
      we    = 1'b1;
      waddr = addr;
      wdata = data;
      @(negedge clk);
      we    = 1'b0;
    end
  endtask

  task automatic read_regs(input logic [AW-1:0] addr_a, input logic [AW-1:0] addr_b);
    begin
      raddr_a = addr_a;
      raddr_b = addr_b;
      #1; // leitura combinacional estabiliza
      $display("t=%0t  R[%0d]=0x%0h | R[%0d]=0x%0h",
               $time, addr_a, rdata_a, addr_b, rdata_b);
    end
  endtask

  // Estímulos
  initial begin
    // Reset inicial
    rst_n   = 1'b0;
    we      = 1'b0;
    waddr   = '0;
    wdata   = '0;
    raddr_a = '0;
    raddr_b = '0;

    repeat (2) @(negedge clk);
    rst_n = 1'b1;

    // 1) Escreve padrão conhecido: R[i] = i * 3
    for (int i = 0; i < DEPTH; i++) begin
      write_reg(i[AW-1:0], WIDTH'(i*3));
    end

    // 2) Leituras paralelas (duas portas)
    read_regs(0, 1);
    read_regs(2, 5);
    read_regs(7, 3);

    // 3) Write-then-read no mesmo endereço
    write_reg(4, 16'hABCD);
    read_regs(4, 7);   // antes da próxima borda útil de escrita

    @(negedge clk);
    read_regs(4, 1);   // após a borda, novo valor visível

    repeat (2) @(negedge clk);
    $finish;
  end

endmodule
