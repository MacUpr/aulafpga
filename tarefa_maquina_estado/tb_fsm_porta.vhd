library ieee;
use ieee.std_logic_1164.all;
use work.porta_definitions_pkg.all;

entity tb_fsm_porta is
end entity tb_fsm_porta;

architecture behavior of tb_fsm_porta is
    constant CLK_PERIOD      : time := 20 ns;
    constant T_ABRINDO_SIM   : time := 200 ms;
    constant T_FECHANDO_SIM  : time := 200 ms;
    constant T_ABERTA_SEGUNDOS_SIM : natural := 3;
    component fsm_porta is
        generic ( CLK_FREQUENCY   : natural; T_ABERTA_SEGUNDOS : natural );
        port ( clk : in std_logic; rst_n : in std_logic; sensor : in std_logic; fechar_manual : in std_logic; fim_curso_aberta : in std_logic; fim_curso_fechada: in std_logic; motor_abrir : out std_logic; motor_fechar : out std_logic; debug_estado : out t_estado );
    end component;
    signal s_clk              : std_logic := '0';
    signal s_rst_n            : std_logic;
    signal s_sensor           : std_logic := '0';
    signal s_fechar_manual    : std_logic := '0';
    signal s_fim_curso_aberta : std_logic := '0';
    signal s_fim_curso_fechada: std_logic := '0';
    signal s_motor_abrir      : std_logic;
    signal s_motor_fechar     : std_logic;
    signal s_debug_estado     : t_estado;
    signal s_debug_estado_vcd : std_logic_vector(2 downto 0);
begin
    UUT: fsm_porta
        generic map ( CLK_FREQUENCY => 1000, T_ABERTA_SEGUNDOS => T_ABERTA_SEGUNDOS_SIM )
        port map ( clk => s_clk, rst_n => s_rst_n, sensor => s_sensor, fechar_manual => s_fechar_manual, fim_curso_aberta => s_fim_curso_aberta, fim_curso_fechada => s_fim_curso_fechada, motor_abrir => s_motor_abrir, motor_fechar => s_motor_fechar, debug_estado => s_debug_estado );

    with s_debug_estado select
        s_debug_estado_vcd <= "001" when FECHADA,
                              "010" when ABRINDO,
                              "100" when ABERTA,
                              "101" when FECHANDO,
                              "000" when others;

    clk_process: process
    begin
        s_clk <= '0'; wait for CLK_PERIOD / 2; s_clk <= '1'; wait for CLK_PERIOD / 2;
    end process;

    stimulus_process: process
    begin
        report "Iniciando..."; s_rst_n <= '0'; wait for 100 ns; s_rst_n <= '1'; wait for CLK_PERIOD;
        report "Cenario 1"; s_sensor <= '1'; wait for CLK_PERIOD; s_sensor <= '0';
        wait for T_ABRINDO_SIM; s_fim_curso_aberta <= '1'; wait for CLK_PERIOD; s_fim_curso_aberta <= '0';
        wait for (T_ABERTA_SEGUNDOS_SIM * 1 sec) + 100 ms;
        wait for T_FECHANDO_SIM; s_fim_curso_fechada <= '1'; wait for CLK_PERIOD; s_fim_curso_fechada <= '0';
        wait for 1 sec;
        report "Cenario 2"; s_sensor <= '1'; wait for CLK_PERIOD; s_sensor <= '0';
        wait for T_ABRINDO_SIM; s_fim_curso_aberta <= '1'; wait for CLK_PERIOD; s_fim_curso_aberta <= '0';
        wait for ( (T_ABERTA_SEGUNDOS_SIM / 2) * 1 sec );
        report "Sensor ativado novamente"; s_sensor <= '1'; wait for 100 ms; s_sensor <= '0';
        wait for (T_ABERTA_SEGUNDOS_SIM * 1 sec) + 100 ms;
        wait for T_FECHANDO_SIM; s_fim_curso_fechada <= '1'; wait for CLK_PERIOD; s_fim_curso_fechada <= '0';
        wait for 1 sec;
        report "Cenario 3"; s_sensor <= '1'; wait for CLK_PERIOD; s_sensor <= '0';
        wait for T_ABRINDO_SIM; s_fim_curso_aberta <= '1'; wait for CLK_PERIOD; s_fim_curso_aberta <= '0';
        wait for 1 sec;
        report "Fechamento manual"; s_fechar_manual <= '1'; wait for CLK_PERIOD; s_fechar_manual <= '0';
        wait for T_FECHANDO_SIM; s_fim_curso_fechada <= '1'; wait for CLK_PERIOD; s_fim_curso_fechada <= '0';
        report "Fim."; wait;
    end process;
end architecture;