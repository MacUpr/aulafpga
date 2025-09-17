library ieee;
use ieee.std_logic_1164.all;

entity tarefa3_TB is
end entity tarefa3_TB;

architecture sim of tarefa3_TB is
    signal A, B, C, D, s : std_logic;
begin
  DUT: entity work.tarefa3
    port map (A => A, B => B, C => C, D => D, s => s);

  stim: process
  begin
     A <= '0'; B <= '0'; C <= '0'; D <= '0'; wait for 10 ns;
     A <= '0'; B <= '1'; C <= '1'; D <= '1'; wait for 10 ns;
     A <= '0'; B <= '0'; C <= '1'; D <= '0'; wait for 10 ns;
     A <= '0'; B <= '0'; C <= '1'; D <= '1'; wait for 10 ns;
     A <= '1'; B <= '1'; C <= '0'; D <= '0'; wait for 10 ns;
     A <= '1'; B <= '0'; C <= '0'; D <= '1'; wait for 10 ns;
     A <= '1'; B <= '1'; C <= '1'; D <= '0'; wait for 10 ns;
     A <= '1'; B <= '1'; C <= '1'; D <= '1'; wait for 10 ns;
    wait;
  end process;
end architecture;
