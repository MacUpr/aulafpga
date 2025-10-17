`timescale 1ns/1ps
`default_nettype none
module top #(
    parameter integer F_CLK_HZ      = 25_000_000, // clock da placa
    parameter integer TICK_HZ       = 1,          // 1 Hz: unidade muda a cada 1 s
    parameter integer ESCALA        = 1,
    parameter integer BLINK_ON_MS   = 500,        // alternância do barramento segue o blink
    parameter integer BLINK_OFF_MS  = 500
)(
    input  wire        clk,
    input  wire        reset,
    input  wire        pause,
    input  wire        dir,
    output wire [8:0]  word_bus,   // <<<< ÚNICO barramento de 9 bits (MSB indica qual palavra)
    output wire        led
);
    // ------------------ sync entradas + reset edge ------------------
    reg [1:0] s_reset, s_pause, s_dir;
    always @(posedge clk) begin
        s_reset <= {s_reset[0], reset};
        s_pause <= {s_pause[0], pause};
        s_dir   <= {s_dir[0],   dir  };
    end
    wire reset_s = s_reset[1];
    wire pause_s = s_pause[1];
    wire dir_s   = s_dir  [1];

    reg reset_s_d;
    always @(posedge clk) reset_s_d <= reset_s;
    wire reset_rise = (reset_s & ~reset_s_d);

    // ------------------ TICK a 1 Hz (contagem) ------------------
    localparam integer TICK_TKS = (ESCALA * F_CLK_HZ / TICK_HZ); // 25MHz/1Hz => 25_000_000
    reg  [31:0] tick_cnt;
    wire        tick = (TICK_TKS > 1) ? (tick_cnt == TICK_TKS-1) : 1'b1;
    always @(posedge clk) begin
        if (reset_rise)                tick_cnt <= 32'd0;
        else if (TICK_TKS > 1)
            tick_cnt <= tick ? 32'd0 : (tick_cnt + 32'd1);
        else
            tick_cnt <= 32'd0;
    end

    // ------------------ BLINK (e seletor do barramento) ------------------
    localparam integer BL_ON_TKS  = (F_CLK_HZ/1000) * BLINK_ON_MS;
    localparam integer BL_OFF_TKS = (F_CLK_HZ/1000) * BLINK_OFF_MS;
    localparam integer BL_PER     = BL_ON_TKS + BL_OFF_TKS;

    reg [31:0] blink_cnt;
    always @(posedge clk) begin
        if (reset_rise)              blink_cnt <= 32'd0;
        else if (BL_PER > 1)
            blink_cnt <= (blink_cnt == BL_PER-1) ? 32'd0 : (blink_cnt + 32'd1);
        else
            blink_cnt <= 32'd0;
    end
    assign led = (blink_cnt < BL_ON_TKS);

    // Seleção do barramento: 0 envia DU, 1 envia MC
    wire sel_hi = (blink_cnt >= BL_ON_TKS); // metade “on” → LO (MSB=0), metade “off” → HI (MSB=1)

    // ------------------ BCD: próximo estado explícito ------------------
    reg [3:0] u, d, c, m;
    reg [3:0] u_n, d_n, c_n, m_n;
    reg       carry0, carry1, carry2;
    reg       borr0,  borr1,  borr2;

    always @* begin
        u_n=u; d_n=d; c_n=c; m_n=m;
        carry0=1'b0; carry1=1'b0; carry2=1'b0;
        borr0 =1'b0; borr1 =1'b0; borr2 =1'b0;

        if (dir_s) begin
            // crescente
            carry0 = (u == 4'd9);               u_n = carry0 ? 4'd0 : (u + 4'd1);
            carry1 = carry0 && (d == 4'd5);     d_n = carry0 ? (carry1 ? 4'd0 : (d + 4'd1)) : d; // dezena 0..5
            carry2 = carry1 && (c == 4'd3);     c_n = carry1 ? (carry2 ? 4'd0 : (c + 4'd1)) : c; // centena 0..3
                                               m_n = carry2 ? ((m == 4'd2) ? 4'd0 : (m + 4'd1)) : m; // milhar 0..2
        end else begin
            // decrescente
            borr0 = (u == 4'd0);                u_n = borr0 ? 4'd9 : (u - 4'd1);
            borr1 = borr0 && (d == 4'd0);       d_n = borr0 ? (borr1 ? 4'd5 : (d - 4'd1)) : d; // 5..0
            borr2 = borr1 && (c == 4'd0);       c_n = borr1 ? (borr2 ? 4'd3 : (c - 4'd1)) : c; // 3..0
                                               m_n = borr2 ? ((m == 4'd0) ? 4'd2 : (m - 4'd1)) : m; // 2..0
        end
    end

    // registra dígitos SOMENTE no tick (1 Hz)
    always @(posedge clk) begin
        if (reset_rise) begin
            u <= 4'd0; d <= 4'd0; c <= 4'd0; m <= 4'd0;
        end else if (tick && !pause_s) begin
            u <= u_n; d <= d_n; c <= c_n; m <= m_n;
        end
    end

    // ------------------ Empacotamento das duas palavras ------------------
    reg [8:0] word_lo_q, word_hi_q;
    always @(posedge clk) begin
        if (reset_rise) begin
            word_lo_q <= 9'b0_0000_0000;   // MSB=0
            word_hi_q <= 9'b1_0000_0000;   // MSB=1
        end else if (tick) begin
            word_lo_q <= {1'b0, d_n, u_n}; // D:U  (0..5 : 0..9)
            word_hi_q <= {1'b1, m_n, c_n}; // M:C  (0..2 : 0..3)
        end
    end

    // ------------------ Único barramento de saída ------------------
    assign word_bus = sel_hi ? word_hi_q : word_lo_q;

endmodule
`default_nettype wire
