`timescale 1ns/1ps
`default_nettype none

// Descomente se pause/up_down forem ATIVOS-BAIXO no pino (pull-up + GND).
//`define BUTTONS_ACTIVE_LOW

module top #(
    parameter int unsigned F_CLK_HZ = 25_000_000,
    parameter int unsigned ON_MS    = 500,
    parameter int unsigned OFF_MS   = 500
)(
    input  wire       clk,       // P3
    input  wire       reset_n,   // B18 (ativo-baixo)
    input  wire       pause,     // D2
    input  wire       up_down,   // B1
    input  wire       load,      // K5 (ATIVO-BAIXO: 0 = carregar preset)
    input  wire [5:0] preset,    // J4,J5,A2,K4,B3,E19
    output wire       led,       // L2
    output wire [5:0] q          // B19..A19
);
    // ----------------------------
    // 1) BLINK + TICK (inalterado)
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
            blink_cnt <= '0;
    end

    wire tick_int = (PERIOD > 1) ? (blink_cnt == PERIOD-1) : 1'b1;
    assign led    = (blink_cnt < ON_TKS);

    // ----------------------------
    // 2) Adaptação de nível dos botões (mantida)
    // ----------------------------
    wire pause_eff;     // 1 = pausa; 0 = libera
    wire up_down_eff;   // 1 = LSB→MSB ; 0 = MSB→LSB
`ifdef BUTTONS_ACTIVE_LOW
    assign pause_eff   = pause;
    assign up_down_eff = ~up_down;
`else
    assign pause_eff   =  ~pause;
    assign up_down_eff =   up_down;
`endif

    // ----------------------------
    // 3) REGISTRADOR DE DESLOCAMENTO CIRCULAR (6b)
    //    Prioridade: reset -> (load==0) -> tick (se !pause)
    //    Reset força 000001; MSB retorna para LSB.
    // ----------------------------
    logic [5:0] q_r = 6'b000001;

    always_ff @(posedge clk or negedge reset_n) begin
        if (!reset_n) begin
            q_r <= 6'b000001;  // palavra final no reset
        end else if (!load) begin
            // load ativo-baixo: carga paralela de preset[5:0]
            q_r <= preset;
        end else if (tick_int && !pause_eff) begin
            // deslocamento circular
            if (up_down_eff) begin
                // LSB→MSB (rotate-left): MSB retorna ao LSB
                q_r <= {q_r[4:0], q_r[5]};
            end else begin
                // MSB→LSB (rotate-right): LSB retorna ao MSB
                q_r <= {q_r[0], q_r[5:1]};
            end
        end
    end

    assign q = q_r;  // saída paralela para acompanhar o deslocamento

endmodule

`default_nettype wire
