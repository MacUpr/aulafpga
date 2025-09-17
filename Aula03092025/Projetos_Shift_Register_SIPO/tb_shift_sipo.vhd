-- Testbench para SIPO Shift Register
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity tb_shift_sipo is
end tb_shift_sipo;

architecture Behavioral of tb_shift_sipo is
    component shift_register_sipo
        port(
            CLK        : in  STD_LOGIC;
            RESET      : in  STD_LOGIC;
            SERIAL_IN  : in  STD_LOGIC;
            PARALLEL_OUT : out STD_LOGIC_VECTOR(7 downto 0)
        );
    end component;
    
    signal CLK_tb        : STD_LOGIC := '0';
    signal RESET_tb      : STD_LOGIC;
    signal SERIAL_IN_tb  : STD_LOGIC;
    signal PARALLEL_OUT_tb : STD_LOGIC_VECTOR(7 downto 0);
    
    constant clock_period : time := 10 ns;
    
    -- Sequência de teste: "10110101"
    type bit_array is array (0 to 7) of STD_LOGIC;
    signal test_sequence : bit_array := ('1','0','1','1','0','1','0','1');
    
begin
    uut: shift_register_sipo port map (
        CLK => CLK_tb,
        RESET => RESET_tb,
        SERIAL_IN => SERIAL_IN_tb,
        PARALLEL_OUT => PARALLEL_OUT_tb
    );
    
    clock_process: process
    begin
        CLK_tb <= '0';
        wait for clock_period/2;
        CLK_tb <= '1';
        wait for clock_period/2;
    end process;
    
    stim_proc: process
    begin
        -- Reset inicial
        RESET_tb <= '1';
        SERIAL_IN_tb <= '0';
        wait for 20 ns;
        
        RESET_tb <= '0';
        wait for 10 ns;
        
        -- Envia sequência bit a bit
        for i in 0 to 7 loop
            SERIAL_IN_tb <= test_sequence(i);
            wait for clock_period;
        end loop;
        
        -- Continua por mais alguns ciclos
        SERIAL_IN_tb <= '0';
        wait for 50 ns;
        
        wait;
    end process;
end Behavioral;
