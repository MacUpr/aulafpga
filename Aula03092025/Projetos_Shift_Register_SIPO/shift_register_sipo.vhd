-- Shift Register SIPO 8-bits
-- Baseado em: Starting Electronics VHDL Tutorial
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity shift_register_sipo is
    port(
        CLK        : in  STD_LOGIC;
        RESET      : in  STD_LOGIC;
        SERIAL_IN  : in  STD_LOGIC;
        PARALLEL_OUT : out STD_LOGIC_VECTOR(7 downto 0)
    );
end shift_register_sipo;

architecture Behavioral of shift_register_sipo is
    signal shift_reg : STD_LOGIC_VECTOR(7 downto 0) := "00000000";
begin
    process(CLK, RESET)
    begin
        if RESET = '1' then
            shift_reg <= "00000000";
        elsif rising_edge(CLK) then
            -- Desloca à direita, novo bit entra pela esquerda
            shift_reg <= SERIAL_IN & shift_reg(7 downto 1);
        end if;
    end process;
    
    PARALLEL_OUT <= shift_reg;
end Behavioral;
