library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity tb_classificador_light is
end tb_classificador_light;

architecture Behavioral of tb_classificador_light is
    -- Componente a ser testado
    component classificador_light
        Port (
            A : in STD_LOGIC;
            B : in STD_LOGIC;
            C : in STD_LOGIC;
            D : in STD_LOGIC;
            S : out STD_LOGIC
        );
    end component;
    
    -- Sinais de teste
    signal A_tb, B_tb, C_tb, D_tb : STD_LOGIC := '0';
    signal S_tb : STD_LOGIC;
    
begin
    -- Instanciação do DUT (Device Under Test)
    DUT: classificador_light port map (
        A => A_tb,
        B => B_tb,
        C => C_tb,
        D => D_tb,
        S => S_tb
    );
    
    -- Processo de teste
    test_process: process
    begin
        -- Teste todas as 16 combinações possíveis
        
        -- Teste 0: 0000 (0%)
        A_tb <= '0'; B_tb <= '0'; C_tb <= '0'; D_tb <= '0';
        wait for 10 ns;
        
        -- Teste 1: 0001 (10%)
        A_tb <= '0'; B_tb <= '0'; C_tb <= '0'; D_tb <= '1';
        wait for 10 ns;
        
        -- Teste 2: 0010 (20%)
        A_tb <= '0'; B_tb <= '0'; C_tb <= '1'; D_tb <= '0';
        wait for 10 ns;
        
        -- Teste 3: 0011 (30%)
        A_tb <= '0'; B_tb <= '0'; C_tb <= '1'; D_tb <= '1';
        wait for 10 ns;
        
        -- Teste 4: 0100 (30%)
        A_tb <= '0'; B_tb <= '1'; C_tb <= '0'; D_tb <= '0';
        wait for 10 ns;
        
        -- Teste 5: 0101 (40%)
        A_tb <= '0'; B_tb <= '1'; C_tb <= '0'; D_tb <= '1';
        wait for 10 ns;
        
        -- Teste 6: 0110 (50%) - exatamente 50%, não acende
        A_tb <= '0'; B_tb <= '1'; C_tb <= '1'; D_tb <= '0';
        wait for 10 ns;
        
        -- Teste 7: 0111 (60%) - acende lâmpada
        A_tb <= '0'; B_tb <= '1'; C_tb <= '1'; D_tb <= '1';
        wait for 10 ns;
        
        -- Teste 8: 1000 (40%)
        A_tb <= '1'; B_tb <= '0'; C_tb <= '0'; D_tb <= '0';
        wait for 10 ns;
        
        -- Teste 9: 1001 (50%) - exatamente 50%, não acende
        A_tb <= '1'; B_tb <= '0'; C_tb <= '0'; D_tb <= '1';
        wait for 10 ns;
        
        -- Teste 10: 1010 (60%) - acende lâmpada
        A_tb <= '1'; B_tb <= '0'; C_tb <= '1'; D_tb <= '0';
        wait for 10 ns;
        
        -- Teste 11: 1011 (70%) - acende lâmpada
        A_tb <= '1'; B_tb <= '0'; C_tb <= '1'; D_tb <= '1';
        wait for 10 ns;
        
        -- Teste 12: 1100 (70%) - acende lâmpada
        A_tb <= '1'; B_tb <= '1'; C_tb <= '0'; D_tb <= '0';
        wait for 10 ns;
        
        -- Teste 13: 1101 (80%) - acende lâmpada
        A_tb <= '1'; B_tb <= '1'; C_tb <= '0'; D_tb <= '1';
        wait for 10 ns;
        
        -- Teste 14: 1110 (90%) - acende lâmpada
        A_tb <= '1'; B_tb <= '1'; C_tb <= '1'; D_tb <= '0';
        wait for 10 ns;
        
        -- Teste 15: 1111 (100%) - acende lâmpada
        A_tb <= '1'; B_tb <= '1'; C_tb <= '1'; D_tb <= '1';
        wait for 10 ns;
        
        wait;
    end process;
end Behavioral;
