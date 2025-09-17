library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity blink_dois_contadores_updown_ap is
    port (
        clk       : in  std_logic;                         -- 25 MHz (SITE "P3")
        rst_n     : in  std_logic;                         -- RESET assíncrono ativo-BAIXO
        preset_n  : in  std_logic;                         -- PRESET assíncrono ativo-BAIXO (carrega metade)
        dir_btn   : in  std_logic;                         -- Botão UP/DOWN (nível: 1=UP, 0=DOWN)
        led       : out std_logic;                         -- LED onboard (SITE "L2")
        count_a   : out std_logic_vector(5 downto 0);      -- contador A (6 bits)
        count_b   : out std_logic_vector(5 downto 0)       -- contador B (6 bits)
    );
end entity;

architecture rtl of blink_dois_contadores_updown_ap is
    -- Blink: alterna LED a cada 1 s (período total 2 s)
    constant F_CLK_HZ     : integer := 25_000_000;
    constant TOGGLE_TICKS : integer := 25_000_000;         -- 1 s @ 25 MHz

    -- Largura automática p/ divisor do blink
    function ulen(n : integer) return integer is
        variable w : integer := 1;
    begin
        while (2**w) < n loop
            w := w + 1;
        end loop;
        return w;
    end function;
    constant W : integer := ulen(TOGGLE_TICKS);

    -- Metade da contagem para 6 bits (0..63) = 32
    constant HALF_6B : unsigned(5 downto 0) := to_unsigned(32, 6);

    -- Divisor e blink
    signal cnt     : unsigned(W-1 downto 0) := (others => '0');
    signal tick    : std_logic;
    signal led_reg : std_logic := '0';

    -- Contadores
    signal reg_a   : unsigned(5 downto 0) := (others => '0');
    signal reg_b   : unsigned(5 downto 0) := (others => '0');

    -- Sincronizador do botão de direção (UP/DOWN)
    signal dir_ff1, dir_ff2 : std_logic := '0';
begin
    -- Pulso de 1 ciclo exatamente no terminal-count
    tick <= '1' when (cnt = to_unsigned(TOGGLE_TICKS-1, W)) else '0';

    process(clk, rst_n, preset_n)
    begin
        -- Prioridade assíncrona: RESET (ativo-baixo) > PRESET (ativo-baixo)
        if rst_n = '0' then
            cnt     <= (others => '0');
            led_reg <= '0';
            reg_a   <= (others => '0');
            reg_b   <= (others => '0');
            dir_ff1 <= '0';
            dir_ff2 <= '0';

        elsif preset_n = '0' then
            -- Carrega metade (32) assincronamente nos contadores
            reg_a   <= HALF_6B;
            reg_b   <= HALF_6B;
            -- (opcional) travar blink durante preset:
            -- cnt     <= (others => '0');
            -- led_reg <= '0';

        elsif rising_edge(clk) then
            -- Sincroniza o botão de direção (evita metastabilidade)
            dir_ff1 <= dir_btn;
            dir_ff2 <= dir_ff1;

            -- Divisor do blink e contagem no MESMO ciclo do toggle
            if tick = '1' then
                cnt     <= (others => '0');
                led_reg <= not led_reg;

                if dir_ff2 = '1' then         -- UP
                    reg_a <= reg_a + 1;
                    reg_b <= reg_b + 1;
                else                           -- DOWN
                    reg_a <= reg_a - 1;       -- wrap-around natural em unsigned
                    reg_b <= reg_b - 1;
                end if;
            else
                cnt <= cnt + 1;
            end if;
        end if;
    end process;

    -- Saídas
    led     <= led_reg;
    count_a <= std_logic_vector(reg_a);
    count_b <= std_logic_vector(reg_b);
end architecture;
