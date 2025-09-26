`include "print_queue.sv"

module top;
  // O top apenas instancia o spooler de impressão.
  print_spooler u_spooler();
endmodule
