// top.v — LED segue o sinal externo, com opção de LED ativo-baixo.
module top
#(
    parameter LED_ACTIVE_LOW = 1  // 0 = ativo-alto; 1 = ativo-baixo (muitas placas)
)
(
    input  wire ext_sig,   // entrada externa (K3)
    output wire led,       // LED blink (segue ext_sig)
    output wire sync_out   // espelha o valor lido
);

    // nível “lógico” do que lemos
    wire v_in = ext_sig;

    // LED com correção de polaridade
    assign led      = (LED_ACTIVE_LOW ? ~v_in : v_in);
    assign sync_out = v_in;

endmodule
