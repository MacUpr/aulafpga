library ieee;
use ieee.std_logic_1164.all;

entity piso is
    port (
        Clk         : in  std_logic;
        reset       : in  std_logic;                      -- Reset assíncrono (ativa alto)
        parallel_in : in  std_logic_vector(3 downto 0);
        load        : in  std_logic;
        shift       : in  std_logic;
        serial_out  : out std_logic;

        -- Sinais internos visíveis externamente
        q0, q1, q2, q3 : out std_logic
    );
end entity;


architecture behavioral of piso is
    signal load_value : std_logic_vector(3 downto 0) := "0000";
begin

    process(Clk, reset)
    begin
        if reset = '1' then
            load_value <= (others => '0');
            serial_out <= '0';
        elsif rising_edge(Clk) then
            if load = '1' then
                load_value <= parallel_in;
            elsif shift = '1' then
                serial_out <= load_value(3);
                load_value(3) <= load_value(2);
                load_value(2) <= load_value(1);
                load_value(1) <= load_value(0);
                load_value(0) <= load_value(3); -- realimentação circular
            end if;
        end if;
    end process;

    -- Conexão dos bits internos às saídas
    q3 <= load_value(3);
    q2 <= load_value(2);
    q1 <= load_value(1);
    q0 <= load_value(0);

end behavioral;

