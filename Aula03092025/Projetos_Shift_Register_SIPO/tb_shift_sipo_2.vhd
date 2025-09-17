library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity tb_shift_sipo is
end tb_shift_sipo;

architecture Behavioral of tb_shift_sipo is
    component shift_register_sipo
        port(
            CLK          : in  STD_LOGIC;
            RESET        : in  STD_LOGIC;
            SERIAL_IN    : in  STD_LOGIC;
            PARALLEL_OUT : out STD_LOGIC_VECTOR(7 downto 0)
        );
    end component;

    -- Sinais para observação no GTKWave
    signal CLK_tb           : STD_LOGIC := '0';
    signal RESET_tb         : STD_LOGIC;
    signal SERIAL_IN_tb     : STD_LOGIC;
    signal PARALLEL_OUT_tb  : STD_LOGIC_VECTOR(7 downto 0);

    -- Alias para bits individuais (para facilitar visualização)
    signal BIT7 : STD_LOGIC;
    signal BIT6 : STD_LOGIC;
    signal BIT5 : STD_LOGIC;
    signal BIT4 : STD_LOGIC;
    signal BIT3 : STD_LOGIC;
    signal BIT2 : STD_LOGIC;
    signal BIT1 : STD_LOGIC;
    signal BIT0 : STD_LOGIC;

    constant clock_period : time := 10 ns;

    type bit_array is array (0 to 7) of STD_LOGIC;
    signal test_sequence : bit_array := ('1','0','1','1','0','1','0','1');

begin
    -- Instância do DUT
    uut: shift_register_sipo port map (
        CLK => CLK_tb,
        RESET => RESET_tb,
        SERIAL_IN => SERIAL_IN_tb,
        PARALLEL_OUT => PARALLEL_OUT_tb
    );

    -- Mapear bits individuais
    BIT7 <= PARALLEL_OUT_tb(7);
    BIT6 <= PARALLEL_OUT_tb(6);
    BIT5 <= PARALLEL_OUT_tb(5);
    BIT4 <= PARALLEL_OUT_tb(4);
    BIT3 <= PARALLEL_OUT_tb(3);
    BIT2 <= PARALLEL_OUT_tb(2);
    BIT1 <= PARALLEL_OUT_tb(1);
    BIT0 <= PARALLEL_OUT_tb(0);

    -- Geração do clock
    clock_process: process
    begin
        while true loop
            CLK_tb <= '0';
            wait for clock_period/2;
            CLK_tb <= '1';
            wait for clock_period/2;
        end loop;
    end process;

    -- Processo de estímulos
    stim_proc: process
    begin
        RESET_tb <= '1';
        SERIAL_IN_tb <= '0';
        wait for 20 ns;

        RESET_tb <= '0';
        wait for 10 ns;

        for i in 0 to 7 loop
            SERIAL_IN_tb <= test_sequence(i);
            wait for clock_period;
        end loop;

        SERIAL_IN_tb <= '0';
        wait for 50 ns;

        wait;
    end process;

end Behavioral;
