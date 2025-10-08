`timescale 1ns/1ps
`default_nettype none

// Descomente se os botões pause/up_down forem ATIVOS-BAIXO no pino (pull-up + GND).
//`define BUTTONS_ACTIVE_LOW

module top #(
    parameter int unsigned F_CLK_HZ = 25_000_000,
    parameter int unsigned ON_MS    = 500,
    parameter int unsigned OFF_MS   = 500
)(
    input  wire       clk,       // clock principal
    input  wire       reset_n,   // reset assíncrono, ATIVO-BAIXO
    input  wire       pause,     // botão de pausa (1 = pressionado, se ativo-alto)
    input  wire       up_down,   // direção: 1 = LSB→MSB (esq), 0 = MSB→LSB (dir)
    output wire       led,       // LED do blink
    output wire [5:0] q          // saída paralela do registrador
);
    // ----------------------------
    // 1) BLINK + TICK internos
    // ----------------------------
    localparam int unsigned TICKS_PER_MS = (F_CLK_HZ / 1000);
    localparam int unsigned ON_TKS  = TICKS_PER_MS * ON_MS;
    localparam int unsigned OFF_TKS = TICKS_PER_MS * OFF_MS;
    localparam int unsigned PERIOD  = ON_TKS + OFF_TKS;
    localparam int unsigned W       = (PERIOD > 1) ? $clog2(PERIOD) : 1;

    logic [W-1:0] blink_cnt = '0;

    always_ff @(posedge clk) begin
        if (PERIOD > 1)
            blink_cnt <= (blink_cnt == PERIOD-1) ? '0 : (blink_cnt + 1'b1);
        else
            blink_cnt <= '0; // período degenerado
    end

    wire tick_int = (PERIOD > 1) ? (blink_cnt == PERIOD-1) : 1'b1;
    assign led    = (blink_cnt < ON_TKS);

    // ----------------------------
    // 2) Sincronização de botões
    // ----------------------------
    logic [1:0] sync_pause  = 2'b00;
    logic [1:0] sync_updown = 2'b00;
    always_ff @(posedge clk) begin
        sync_pause  <= {sync_pause[0],  pause};
        sync_updown <= {sync_updown[0], up_down};
    end
    wire pause_s   = sync_pause[1];
    wire up_down_s = sync_updown[1];

    // Mapeamento lógico (pause_eff=1 => pausado; up_down_eff=1 => LSB→MSB)
`ifdef BUTTONS_ACTIVE_LOW
    wire pause_eff   = pause_s;    // ativo-baixo: pressionado(0) -> 1 (pausa)
    wire up_down_eff = ~up_down_s;  // ativo-baixo: pressionado(0) -> 1 (LSB→MSB)
`else
    wire pause_eff   =  ~pause_s;    // ativo-alto: pressionado(1) -> 1 (pausa)
    wire up_down_eff =  up_down_s;  // ativo-alto: 1 => LSB→MSB
`endif

    // ----------------------------
    // 3) REGISTRADOR DE DESLOCAMENTO CIRCULAR (6 bits)
    //    Reset força 000001.
    //    Direção:
    //      - up_down_eff=1: LSB→MSB (rotaciona à esquerda)  MSB volta para LSB.
    //      - up_down_eff=0: MSB→LSB (rotaciona à direita)  LSB volta para MSB.
    // ----------------------------
    logic [5:0] q_r = 6'b000001;

    always_ff @(posedge clk or negedge reset_n) begin
        if (!reset_n) begin
            q_r <= 6'b000001; // palavra final exigida no reset
        end else if (tick_int && !pause_eff) begin
            if (up_down_eff) begin
                // LSB -> MSB (rotate-left): {b4..b0, b5}; MSB retorna ao LSB
                q_r <= {q_r[4:0], q_r[5]};
            end else begin
                // MSB -> LSB (rotate-right): {b0, b5..b1}; LSB retorna ao MSB
                q_r <= {q_r[0], q_r[5:1]};
            end
        end
    end

    assign q = q_r; // saída paralela para acompanhamento

endmodule

`default_nettype wire
