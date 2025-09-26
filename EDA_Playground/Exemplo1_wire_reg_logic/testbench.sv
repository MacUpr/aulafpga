module tb;
  // Estímulos
  logic a, b, sel, clk, rst_n;
  // Observações
  logic y_comb, y_mux, q_dff;

  // DUT
  top dut(
    .a(a), .b(b), .sel(sel),
    .clk(clk), .rst_n(rst_n),
    .y_comb(y_comb), .y_mux(y_mux), .q_dff(q_dff)
  );

  // Clock 100 MHz (período 10 ns)
  initial clk = 1'b0;
  always  #5 clk = ~clk;

  // Dump para EPWave/GTKWave
  initial begin
    $dumpfile("waveform.vcd");
    $dumpvars(0, tb);
  end

  // Estímulos e logs
  initial begin
    rst_n = 0; a = 0; b = 0; sel = 0;
    #12 rst_n = 1;

    repeat (6) begin
      #7  a   = $urandom_range(0,1);
          b   = $urandom_range(0,1);
          sel = $urandom_range(0,1);
      #3  $display("t=%0t | a=%0b b=%0b sel=%0b || y_comb=%0b y_mux=%0b q_dff=%0b",
                   $time, a, b, sel, y_comb, y_mux, q_dff);
    end

    #20 $finish;
  end
endmodule
