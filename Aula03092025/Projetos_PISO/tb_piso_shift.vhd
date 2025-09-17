library ieee;
use ieee.std_logic_1164.all;

entity tb_piso_shift is
end entity;

architecture sim of tb_piso_shift is
  signal Clk         : std_logic := '0';
  signal load        : std_logic := '0';
  signal shift       : std_logic := '0';
  signal parallel_in : std_logic_vector(3 downto 0) := (others => '0');
  signal serial_out  : std_logic;

  component piso
    port (
      Clk         : in  std_logic;
      parallel_in : in  std_logic_vector(3 downto 0);
      load        : in  std_logic;
      shift       : in  std_logic;
      serial_out  : out std_logic
    );
  end component;

begin

  uut: piso port map(
    Clk         => Clk,
    parallel_in => parallel_in,
    load        => load,
    shift       => shift,
    serial_out  => serial_out
  );

  -- Clock de 10 ns
  clk_process: process
  begin
    while now < 200 ns loop
      Clk <= not Clk;
      wait for 5 ns;
    end loop;
    wait;
  end process;

  -- Estímulos
  stim_proc: process
  begin
    -- Espera inicial
    wait for 10 ns;

    -- Carga paralela
    parallel_in <= "1011"; -- d(3)=1, d(2)=0, d(1)=1, d(0)=1
    load <= '1';
    shift <= '0';
    wait for 10 ns;

    load <= '0';
    wait for 10 ns;

    -- Deslocamento serial habilitado
    shift <= '1';
    for i in 0 to 3 loop
      wait for 10 ns;
    end loop;

    -- Finaliza
    shift <= '0';
    wait;
  end process;

end architecture;
