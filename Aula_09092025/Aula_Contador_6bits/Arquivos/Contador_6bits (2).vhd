library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity blink_dois_contadores is
    Port (
        clk      : in  std_logic;                          -- 25 MHz (SITE "P3")
        led      : out std_logic;                          -- LED onboard (SITE "L2")
        count_a  : out std_logic_vector(5 downto 0);       -- contador A (6 bits)
        count_b  : out std_logic_vector(5 downto 0)        -- contador B (6 bits)
    );
end blink_dois_contadores;

architecture Behavioral of blink_dois_contadores is
    -- LED com período TOTAL de 2 s  => meia-período (toggle) = 1 s
    constant F_CLK_HZ      : integer := 25_000_000;        -- clock de entrada
    constant TOGGLE_TICKS  : integer := 25_000_000;        -- 1 s @ 25 MHz
    constant TOGGLE_MAX    : unsigned(24 downto 0) := to_unsigned(TOGGLE_TICKS-1, 25);

    signal cnt     : unsigned(24 downto 0) := (others => '0');  -- conta 0..(TOGGLE_TICKS-1)
    signal tick    : std_logic;                                -- 1 ciclo no instante do toggle
    signal led_reg : std_logic := '0';

    signal reg_a   : unsigned(5 downto 0) := (others => '0');  -- contador A
    signal reg_b   : unsigned(5 downto 0) := (others => '0');  -- contador B
begin
    -- tick = '1' exatamente quando atingimos o terminal-count
    tick <= '1' when (cnt = TOGGLE_MAX) else '0';

    process(clk)
    begin
        if rising_edge(clk) then
            if tick = '1' then
                cnt     <= (others => '0');      -- reinicia o divisor
                led_reg <= not led_reg;          -- pisca o LED

                -- >>> contadores incrementam no MESMO ciclo do toggle
                reg_a   <= reg_a + 1;
                reg_b   <= reg_b + 1;
            else
                cnt     <= cnt + 1;
            end if;
        end if;
    end process;

    -- saídas
    led     <= led_reg;
    count_a <= std_logic_vector(reg_a);
    count_b <= std_logic_vector(reg_b);
end Behavioral;
