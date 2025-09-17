library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity classificador_light is
    Port (
        A : in STD_LOGIC;   -- Ingrediente A (40%)
        B : in STD_LOGIC;   -- Ingrediente B (30%)
        C : in STD_LOGIC;   -- Ingrediente C (20%)
        D : in STD_LOGIC;   -- Ingrediente D (10%)
        S : out STD_LOGIC   -- Saída (lâmpada)
    );
end classificador_light;

architecture Behavioral of classificador_light is
begin
    -- Implementação da lógica simplificada: S = A(B + C)
    S <= A and (B or C);
end Behavioral;
