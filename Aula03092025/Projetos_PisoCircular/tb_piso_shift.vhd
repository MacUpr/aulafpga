library ieee;
use ieee.std_logic_1164.all;

entity tb_piso_shift is
end entity;

architecture sim of tb_piso_shift is
  -- Sinais principais
  signal Clk         : std_logic := '0';
  signal reset       : std_logic := '0';
  signal load        : std_logic := '0';
  signal shift       : std_logic := '0';
  signal parallel_in : std_logic_vector(3 downto 0) := (others => '0');
  signal serial_out  : std_logic;

  -- Sinais internos (bits individuais do registrador)
  signal q0, q1, q2, q3 : std_logic;

  component piso
    port (
      Clk         : in  std_logic;
      reset       : in  std_logic;
      parallel_in : in  std_logic_vector(3 downto 0);
      load        : in  std_logic;
      shift       : in  std_logic;
      serial_out  : out std_logic;
      q0, q1, q2, q3 : out std_logic
    );
  end component;

begin

  -- Instância do componente PISO
  uut: piso port map(
    Clk         => Clk,
    reset       => reset,
    parallel_in => parallel_in,
    load        => load,
    shift       => shift,
    serial_out  => serial_out,
    q0 => q0, q1 => q1, q2 => q2, q3 => q3
  );

  -- Geração de clock: 10 ns (50 MHz)
  clk_process: process
  begin
    while now < 300 ns loop
      Clk <= not Clk;
      wait for 5 ns;
    end loop;
    wait;
  end process;

  -- Estímulos para teste
  stim_proc: process
  begin
    -- Reset global
    reset <= '1';
    wait for 10 ns;
    reset <= '0';

    wait for 10 ns;

    -- Carrega o valor paralelo: 1011
    parallel_in <= "1011"; -- q3=1, q2=0, q1=1, q0=1
    load <= '1';
    shift <= '0';
    wait for 10 ns;

    -- Finaliza carga
    load <= '0';
    wait for 10 ns;

    -- Habilita deslocamento circular por 10 ciclos (~100 ns)
    shift <= '1';
    wait for 250 ns;

    -- Desativa deslocamento
    shift <= '0';
    wait;

  end process;

end architecture;
