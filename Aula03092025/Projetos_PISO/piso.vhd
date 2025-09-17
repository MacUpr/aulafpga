library ieee;
use ieee.std_logic_1164.all;

entity piso is
    port (
        Clk         : in  std_logic;                      -- Clock
        parallel_in : in  std_logic_vector(3 downto 0);   -- Dados de entrada paralela
        load        : in  std_logic;                      -- Carregamento paralelo (ativa alto)
        shift       : in  std_logic;                      -- Habilita deslocamento serial (ativa alto)
        serial_out  : out std_logic                       -- Saída serial
    );
end piso;

architecture behavioral of piso is
    signal load_value : std_logic_vector(3 downto 0) := "0000";
begin

    process(Clk)
    begin
        if rising_edge(Clk) then
            if load = '1' then
                -- Carregamento paralelo
                load_value <= parallel_in;
            elsif shift = '1' then
                -- Deslocamento somente se habilitado
                serial_out <= load_value(3);              -- MSB na saída
                load_value(3) <= load_value(2);
                load_value(2) <= load_value(1);
                load_value(1) <= load_value(0);
                load_value(0) <= '0';                    -- LSB limpo
            end if;
            -- Caso contrário: mantém os valores
        end if;
    end process;

end behavioral;
