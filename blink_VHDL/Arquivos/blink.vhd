library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity blink is
  port(
    clk : in  std_logic;
    led : out std_logic
  );
end entity;

architecture rtl of blink is
  signal counter : unsigned(31 downto 0) := (others => '0');
  signal led_r   : std_logic := '0';
  constant MAX_COUNT : unsigned(31 downto 0) := to_unsigned(12_500_000-1, 32);
begin
  process(clk)
  begin
    if rising_edge(clk) then
      if counter < MAX_COUNT then
        counter <= counter + 1;
      else
        counter <= (others => '0');
        led_r   <= not led_r;
      end if;
    end if;
  end process;
  led <= led_r;
end architecture;
