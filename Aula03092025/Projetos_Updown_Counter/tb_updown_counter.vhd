-- Testbench para Contador Up-Down
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity tb_updown_counter is
end tb_updown_counter;

architecture Behavioral of tb_updown_counter is
    component updown_counter
        port(
            clk     : in  STD_LOGIC;
            reset   : in  STD_LOGIC;
            up_down : in  STD_LOGIC;
            counter : out STD_LOGIC_VECTOR(3 downto 0)
        );
    end component;
    
    signal clk_tb     : STD_LOGIC := '0';
    signal reset_tb   : STD_LOGIC;
    signal up_down_tb : STD_LOGIC;
    signal counter_tb : STD_LOGIC_VECTOR(3 downto 0);
    
    constant clock_period : time := 10 ns;
    
begin
    uut: updown_counter port map (
        clk => clk_tb,
        reset => reset_tb,
        up_down => up_down_tb,
        counter => counter_tb
    );
    
    clock_process: process
    begin
        clk_tb <= '0';
        wait for clock_period/2;
        clk_tb <= '1';
        wait for clock_period/2;
    end process;
    
    stim_proc: process
    begin
        -- Reset inicial
        reset_tb <= '1';
        up_down_tb <= '0';
        wait for 20 ns;
        
        reset_tb <= '0';
        wait for 10 ns;
        
        -- Conta para cima
        up_down_tb <= '0';
        wait for 200 ns;  -- 20 ciclos
        
        -- Conta para baixo
        up_down_tb <= '1';
        wait for 300 ns;  -- 30 ciclos
        
        -- Teste de reset durante operação
        reset_tb <= '1';
        wait for 20 ns;
        reset_tb <= '0';
        
        -- Continua contando para cima
        up_down_tb <= '0';
        wait for 100 ns;
        
        wait;
    end process;
end Behavioral;
