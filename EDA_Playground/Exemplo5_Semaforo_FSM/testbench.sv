
module tb;
  
  // Sinais do DUT
  logic clk, rst_n;
  logic red, yellow, green;
  logic [1:0] state;
  
  // Estados definidos localmente (mesmos valores do módulo)
  localparam logic [1:0] S_RED    = 2'b00;
  localparam logic [1:0] S_GREEN  = 2'b01;
  localparam logic [1:0] S_YELLOW = 2'b10;
  
  // Instancia o design integrador (top) e
  // conecta explicitamente os sinais à instância u_fsm
  top dut();
  
  // Conexões explícitas aos pinos não amarrados do 'top'
  // (o 'top' apenas instanciou a FSM; aqui ligamos sinais)
  assign dut.u_fsm.clk    = clk;
  assign dut.u_fsm.rst_n  = rst_n;
  assign red               = dut.u_fsm.red;
  assign yellow            = dut.u_fsm.yellow;
  assign green             = dut.u_fsm.green;
  assign state             = dut.u_fsm.state;
  
  // Geração de clock (100 MHz -> período 10 ns)
  initial clk = 1'b0;
  always  #5 clk = ~clk;
  
  // Waveform para EPWave/GTKWave
  initial begin
    $dumpfile("waveform.vcd");
    $dumpvars(0, tb);
  end
  
  // Função utilitária para imprimir o nome do estado
  function string state_name(input logic [1:0] s);
    case (s)
      S_RED:    return "RED";
      S_GREEN:  return "GREEN";
      S_YELLOW: return "YELLOW";
      default:  return "UNKNOWN";
    endcase
  endfunction
  
  // Monitora transições de estado e tempos
  logic [1:0] last_state;
  int         dwell;
  
  initial begin
    // Reset inicial
    rst_n      = 1'b0;
    last_state = 2'bxx;
    dwell      = 0;
    repeat (3) @(negedge clk);
    rst_n = 1'b1;
    
    // Roda por alguns ciclos suficientes para observar várias voltas
    repeat (60) begin
      @(posedge clk);
      if (state !== last_state) begin
        // Ao detectar mudança, relata quanto tempo o estado anterior durou
        if (last_state !== 2'bxx) begin
          $display("t=%0t  leave %-6s after %0d cycles",
                   $time, state_name(last_state), dwell);
        end
        $display("t=%0t  enter %-6s  (R=%0b Y=%0b G=%0b)",
                 $time, state_name(state), red, yellow, green);
        last_state = state;
        dwell      = 1;
      end else begin
        dwell++;
      end
    end
    $finish;
  end
  
endmodule