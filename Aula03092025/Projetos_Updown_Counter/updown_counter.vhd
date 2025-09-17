-- Contador Up-Down 4-bits
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity updown_counter is
    port(
        clk     : in  STD_LOGIC;
        reset   : in  STD_LOGIC;
        up_down : in  STD_LOGIC;  -- '1' para down, '0' para up
        counter : out STD_LOGIC_VECTOR(3 downto 0)
    );
end updown_counter;

architecture Behavioral of updown_counter is
    signal counter_updown : unsigned(3 downto 0);
begin
    process(clk)
    begin
        if rising_edge(clk) then
            if reset = '1' then
                counter_updown <= (others => '0');
            elsif up_down = '1' then
                counter_updown <= counter_updown - 1;  -- count down
            else
                counter_updown <= counter_updown + 1;  -- count up
            end if;
        end if;
    end process;
    
    counter <= std_logic_vector(counter_updown); -- conversão para saída
end Behavioral;
