

// FSM de semáforo veicular (Moore) com tempos parametrizados
module traffic_light #(
  parameter int T_GREEN  = 12,
  parameter int T_YELLOW = 4,
  parameter int T_RED    = 10
)(
  input  logic clk,
  input  logic rst_n,
  output logic red,
  output logic yellow,
  output logic green,
  output logic [1:0] state
);
  
  // Estados definidos como parâmetros locais (em vez de enum)
  localparam logic [1:0] S_RED    = 2'b00;
  localparam logic [1:0] S_GREEN  = 2'b01;
  localparam logic [1:0] S_YELLOW = 2'b10;
  
  logic [1:0] curr, next;
  int unsigned tick;  // contador de tempo dentro do estado
  
  // Saídas Moore: dependem apenas do estado
  always_comb begin
    red    = (curr == S_RED);
    yellow = (curr == S_YELLOW);
    green  = (curr == S_GREEN);
  end
  
  // Próximo estado e controle de término de tempo
  // Quando 'tick' atinge (T_* - 1), avança para o próximo estado
  always_comb begin
    next = curr;
    case (curr)
      S_GREEN:  next = (tick == T_GREEN  - 1) ? S_YELLOW : S_GREEN;
      S_YELLOW: next = (tick == T_YELLOW - 1) ? S_RED    : S_YELLOW;
      S_RED:    next = (tick == T_RED    - 1) ? S_GREEN  : S_RED;
      default:  next = S_RED;
    endcase
  end
  
  // Estado e temporização (sequencial)
  always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
      curr <= S_RED;
      tick <= 0;
    end else begin
      if (next != curr) begin
        curr <= next;
        tick <= 0;           // ao mudar de estado, zera o contador
      end else begin
        tick <= tick + 1;    // segue contando dentro do mesmo estado
      end
    end
  end
  
  // Exporta o estado atual para observação externa
  assign state = curr;
  
endmodule

