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
    input  wire       pause,     // botão de pausa
    input  wire       up_down,   // direção: 1=sobe, 0=desce
    output wire       led,       // LED do blink
    output wire [5:0] q          // contador 6 bits
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
    // 2) Adaptação dos botões
    // ----------------------------
    wire pause_eff;     // 1 = pausa; 0 = libera
    wire up_down_eff;   // 1 = sobe ; 0 = desce
`ifdef BUTTONS_ACTIVE_LOW
    assign pause_eff   = pause;     // pressionado=0 -> 1 (pausa)
    assign up_down_eff = ~up_down;   // pressionado=0 -> 1 (sobe)
`else
    assign pause_eff   =  ~pause;
    assign up_down_eff =  up_down;
`endif

    // ----------------------------
    // 3) CONTADOR 6 bits
    //    Prioridade: reset -> tick
    // ----------------------------
    logic [5:0] q_r = '0;

    always_ff @(posedge clk or negedge reset_n) begin
        if (!reset_n) begin
            q_r <= '0;
        end else if (tick_int && !pause_eff) begin
            q_r <= up_down_eff ? (q_r + 6'd1) : (q_r - 6'd1);
        end
    end

    assign q = q_r;

endmodule

`default_nettype wire
