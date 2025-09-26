`include "traffic_light.sv"
module top;
  // Instancia o semáforo com tempos padrão (em ciclos de clock)
  // Ex.:  GREEN=12, YELLOW=4, RED=10 — ajuste livre no TB se desejar
  traffic_light #(
    .T_GREEN (12),
    .T_YELLOW(4),
    .T_RED   (10)
  ) u_fsm (
    .clk   (),
    .rst_n (),
    .red   (),
    .yellow(),
    .green (),
    .state ()
  );
endmodule
