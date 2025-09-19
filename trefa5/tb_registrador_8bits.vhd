library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity tb_registrador_8bits is
end tb_registrador_8bits;

architecture Behavioral of tb_registrador_8bits is
    -- Declaração do componente a ser testado
    component registrador_8bits
        Port ( 
            D   : in  STD_LOGIC_VECTOR(7 downto 0);
            CLK : in  STD_LOGIC;
            Q   : out STD_LOGIC_VECTOR(7 downto 0);
            Q_n : out STD_LOGIC_VECTOR(7 downto 0)
        );
    end component;
    
    -- Sinais de teste
    signal D_test   : STD_LOGIC_VECTOR(7 downto 0) := (others => '0');
    signal CLK_test : STD_LOGIC := '0';
    signal Q_test   : STD_LOGIC_VECTOR(7 downto 0);
    signal Q_n_test : STD_LOGIC_VECTOR(7 downto 0);
    
    -- Período do clock
    constant CLK_PERIOD : time := 10 ns;
    -- Número de ciclos de clock para simular (ajuste para reduzir tamanho do VCD)
    constant NUM_CYCLES : natural := 16; 
    
begin
    -- Instanciação do registrador
    UUT: registrador_8bits port map (
        D   => D_test,
        CLK => CLK_test,
        Q   => Q_test,
        Q_n => Q_n_test
    );
    
    -- Clock de 10 ns
    clk_process: process
    begin
        for i in 0 to 20 loop
            CLK_test <= '0'; wait for 5 ns;
            CLK_test <= '1'; wait for 5 ns;
        end loop;
        wait; -- Para finalizar a simulação
    end process;

    -- Estímulos (os 8 valores solicitados)
    stim_process: process
    begin
        -- Teste 1: 00000000
        D_test <= "00000000"; wait for 10 ns;
        -- Teste 2: 11111111
        D_test <= "11111111"; wait for 10 ns;
        -- Teste 3: 10101010
        D_test <= "10101010"; wait for 10 ns;
        -- Teste 4: 01010101
        D_test <= "01010101"; wait for 10 ns;
        -- Teste 5: 11001100
        D_test <= "11001100"; wait for 10 ns;
        -- Teste 6: 00110011
        D_test <= "00110011"; wait for 10 ns;
        -- Teste 7: 11110000
        D_test <= "11110000"; wait for 10 ns;
        -- Teste 8: 00001111
        D_test <= "00001111"; wait for 10 ns;

        wait; -- Finaliza a simulação
    end process;
    
end Behavioral;