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
    // 1) BLINK + TICK
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
    // 2) Adaptação de nível dos botões
    // ----------------------------
    wire pause_eff;     // 1 = pausa; 0 = libera
    wire up_down_eff;   // 1 = sobe ; 0 = desce
`ifdef BUTTONS_ACTIVE_LOW
    assign pause_eff   = pause;
    assign up_down_eff = ~up_down;
`else
    assign pause_eff   =  ~pause;
    assign up_down_eff =  up_down;
`endif

    // ----------------------------
    // 3) CONTADOR em 7 bits (unsigned) + saída 6 bits
    //    Prioridade: reset -> (load==0) -> tick
    //    Obs.: usar 7 bits garante carry na virada 31→32.
    // ----------------------------
    logic [6:0] cnt7 = 7'd0;  // contador base (0..127)

    always_ff @(posedge clk or negedge reset_n) begin
        if (!reset_n) begin
            cnt7 <= 7'd0;

        end else if (!load) begin
            // load ativo-baixo: carrega preset (zero-extend p/ 7 bits)
            cnt7 <= {1'b0, preset};

        end else if (tick_int && !pause_eff) begin
            if (up_down_eff) begin
                // sobe: +1 (mod 128) → q pega [5:0] e vira mod 64
                cnt7 <= cnt7 + 7'd1;
            end else begin
                // desce: -1 ≡ +127 (mod 128) → q pega [5:0] e vira mod 64
                cnt7 <= cnt7 + 7'd127;
            end
        end
    end

    assign q = cnt7[5:0];  // só os 6 LSBs são exportados

endmodule

`default_nettype wire
