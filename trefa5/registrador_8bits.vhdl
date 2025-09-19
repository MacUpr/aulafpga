library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity registrador_8bits is
    Port ( 
        D   : in  STD_LOGIC_VECTOR(7 downto 0);  -- Entrada paralela de 8 bits
        CLK : in  STD_LOGIC;                     -- Clock comum
        Q   : out STD_LOGIC_VECTOR(7 downto 0);  -- Saída Q de 8 bits
        Q_n : out STD_LOGIC_VECTOR(7 downto 0)   -- Saída Q' de 8 bits
    );
end registrador_8bits;

architecture Structural of registrador_8bits is
begin
    -- Instantiate 8 D flip-flops using a generate loop and direct entity instantiation
    gen_ff: for i in 0 to 7 generate
        FF: entity work.flipflop_d port map (
            D   => D(i),
            CLK => CLK,
            Q   => Q(i),
            Q_n => Q_n(i)
        );
    end generate gen_ff;

end Structural;
