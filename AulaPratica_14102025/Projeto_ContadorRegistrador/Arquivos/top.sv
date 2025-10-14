`timescale 1ns/1ps
`default_nettype none
// ============================================================
// HÍBRIDO 9b — Contador + Registrador de Deslocamento + BLINK
// - RESET (borda de subida): contador=000000000; registrador=000000001
// - LOAD  (borda de subida): carrega ambos com data_in[8:0]
// - PAUSE (nível 1): pausa evolução (tick ignora)
// - DIR   (nível 1): contador sobe; registrador LSB→MSB (rotate-left)
//                    (nível 0): contador desce; registrador MSB→LSB (rotate-right)
// - MODO  (nível 1): saída = CONTADOR; (nível 0): saída = REGISTRADOR
// - BLINK: led fica alto durante ON_MS em cada período (ON_MS+OFF_MS)
// ============================================================
module top #(
    parameter int unsigned F_CLK_HZ = 25_000_000,
    parameter int unsigned ON_MS    = 500,
    parameter int unsigned OFF_MS   = 500
)(
    input  wire        clk,        // clock
    input  wire        reset,      // RESET: ativo em BORDA DE SUBIDA (1)
    input  wire        load,       // LOAD : ativo em BORDA DE SUBIDA (1)
    input  wire        pause,      // PAUSE: ativo em nível alto   (1)
    input  wire        dir,        // DIREÇÃO: 1=up / LSB→MSB; 0=down / MSB→LSB
    input  wire        modo,       // MODO: 1=contador; 0=registrador
    input  wire [8:0]  data_in,    // palavra de entrada para LOAD
    output wire [8:0]  q,          // palavra de saída (selecionada pelo modo)
    output wire        led         // blink visível
);

    // --------------------------------------------------------
    // 1) Blink/Tick interno
    // --------------------------------------------------------
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
            blink_cnt <= '0; // período degenerado (tick constante)
    end

    wire tick_int = (PERIOD > 1) ? (blink_cnt == PERIOD-1) : 1'b1;
    assign led    = (blink_cnt < ON_TKS);

    // --------------------------------------------------------
    // 2) Sincronização + detecção de borda (reset/load)
    // --------------------------------------------------------
    logic [1:0] s_reset = 2'b00, s_load = 2'b00;
    logic [1:0] s_pause = 2'b00, s_dir  = 2'b00, s_modo = 2'b00;

    always_ff @(posedge clk) begin
        s_reset <= {s_reset[0], reset};
        s_load  <= {s_load[0],  load };
        s_pause <= {s_pause[0], pause};
        s_dir   <= {s_dir[0],   dir  };
        s_modo  <= {s_modo[0],  modo };
    end

    wire reset_s = s_reset[1];
    wire load_s  = s_load [1];
    wire pause_s = s_pause[1];
    wire dir_s   = s_dir  [1];
    wire modo_s  = s_modo [1];

    // bordas de subida
    logic reset_s_d = 1'b0, load_s_d = 1'b0;
    always_ff @(posedge clk) begin
        reset_s_d <= reset_s;
        load_s_d  <= load_s;
    end
    wire reset_rise = (reset_s & ~reset_s_d);
    wire load_rise  = (load_s  & ~load_s_d );

    // --------------------------------------------------------
    // 3) Estados internos
    // --------------------------------------------------------
    logic [9:0] cnt10 = 10'd0;         // contador base (mod 1024)
    logic [8:0] reg9  = 9'b000000001;  // registrador circular

    // --------------------------------------------------------
    // 4) Atualização com prioridades
    //    RESET↑ → LOAD↑ → (tick_int & !pause)
    // --------------------------------------------------------
    always_ff @(posedge clk) begin
        if (reset_rise) begin
            cnt10 <= 10'd0;
            reg9  <= 9'b000000001;

        end else if (load_rise) begin
            cnt10 <= {1'b0, data_in};  // zero-extend
            reg9  <= data_in;

        end else if (tick_int && !pause_s) begin
            // contador
            if (dir_s) begin
                cnt10 <= cnt10 + 10'd1;
            end else begin
                cnt10 <= cnt10 + 10'd1023; // -1 mod 1024
            end
            // registrador circular
            if (dir_s) begin
                // rotate-left: {b7..b0, b8}
                reg9 <= {reg9[7:0], reg9[8]};
            end else begin
                // rotate-right: {b0, b8..b1}
                reg9 <= {reg9[0], reg9[8:1]};
            end
        end
    end

    // --------------------------------------------------------
    // 5) MUX de saída conforme MODO
    // --------------------------------------------------------
    assign q = modo_s ? cnt10[8:0] : reg9;

endmodule
`default_nettype wire
