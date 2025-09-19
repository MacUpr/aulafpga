library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity flipflop_d is
    Port ( 
        D   : in  STD_LOGIC;  -- Entrada de dados
        CLK : in  STD_LOGIC;  -- Entrada de clock
        Q   : out STD_LOGIC;  -- Saída Q
        Q_n : out STD_LOGIC   -- Saída Q' (negada)
    );
end flipflop_d;

architecture Behavioral of flipflop_d is
    signal q_internal : STD_LOGIC := '0';
begin
    process(CLK)
    begin
        if rising_edge(CLK) then  -- Detecção de borda ascendente
            q_internal <= D;
        end if;
    end process;
    
    Q <= q_internal;      -- Saída Q
    Q_n <= not q_internal; -- Saída Q' (complemento)
end Behavioral;