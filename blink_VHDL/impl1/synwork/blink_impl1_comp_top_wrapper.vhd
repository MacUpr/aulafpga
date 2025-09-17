--
-- Synopsys
-- Vhdl wrapper for top level design, written on Tue Sep  2 11:23:58 2025
--
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity wrapper_for_blink is
   port (
      clk : in std_logic;
      led : out std_logic
   );
end wrapper_for_blink;

architecture rtl of wrapper_for_blink is

component blink
 port (
   clk : in std_logic;
   led : out std_logic
 );
end component;

signal tmp_clk : std_logic;
signal tmp_led : std_logic;

begin

tmp_clk <= clk;

led <= tmp_led;



u1:   blink port map (
		clk => tmp_clk,
		led => tmp_led
       );
end rtl;
