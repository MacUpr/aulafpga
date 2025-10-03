library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- Local package defining the finite-state machine states for the door.
-- Declared locally so this file is self-contained and does not depend on
-- an external package file. The state names are in Portuguese:
-- FECHADA  : door closed
-- ABRINDO  : door opening
-- ABERTA   : door open
-- FECHANDO : door closing
package porta_local_pkg is
    type t_estado is (FECHADA, ABRINDO, ABERTA, FECHANDO);
end package porta_local_pkg;

-- Make the local package visible to the rest of this file
use work.porta_local_pkg.all;

entity fsm_porta is
    generic (
        CLK_FREQUENCY   : natural := 50_000_000;
        T_ABERTA_SEGUNDOS : natural := 5
    );
    port (
        clk              : in  std_logic;
        rst_n            : in  std_logic;  -- active-low reset
        sensor           : in  std_logic;  -- presence sensor (triggers opening)
        fechar_manual    : in  std_logic;  -- manual command to close
        fim_curso_aberta : in  std_logic;  -- end-of-travel: fully open
        fim_curso_fechada: in  std_logic;  -- end-of-travel: fully closed
        motor_abrir      : out std_logic;  -- open motor drive (active high)
        motor_fechar     : out std_logic;  -- close motor drive (active high)
        -- debug output exposing the current state; useful for simulation
        -- or connecting to LEDs. Note it's an enumerated type.
        debug_estado     : out t_estado
    );
end entity fsm_porta;

architecture rtl of fsm_porta is
    -- Current and next-state registers for the FSM
    signal estado_atual, proximo_estado : t_estado;

    -- Maximum count for the "open" timer. When the door is in the ABERTA
    -- state this counter counts clock cycles up to MAX_CONTAGEM which is
    -- CLK_FREQUENCY * T_ABERTA_SEGUNDOS (i.e. number of clock cycles in
    -- T_ABERTA_SEGUNDOS seconds). This implements a timeout while the
    -- door remains open.
    constant MAX_CONTAGEM : natural := CLK_FREQUENCY * T_ABERTA_SEGUNDOS;

    -- Small counter used as the open-state timer. Range constrained to
    -- avoid unbounded natural growth; reset logic keeps it within range.
    signal contador       : natural range 0 to MAX_CONTAGEM;

    -- One-hot style signal indicating the timer expired (used in combinational
    -- next-state logic). Represented as '1' or '0' for simplicity.
    signal timer_expirou  : std_logic;
begin
    -- Expose internal state for debugging/monitoring
    debug_estado <= estado_atual;

    -- Synchronous process: state register and timer update.
    -- Sensitivity includes rst_n for asynchronous reset handling.
    process(clk, rst_n)
    begin
        -- Active-low reset: return to FECHADA (closed) and clear timer
        if rst_n = '0' then
            estado_atual <= FECHADA;
            contador     <= 0;
        elsif rising_edge(clk) then
            -- Update the state register with the previously computed
            -- next state (standard two-process FSM style).
            estado_atual <= proximo_estado;

            -- Timer behaviour: only count while the next state is ABERTA.
            -- This means the counter increments while the door is open.
            -- If the sensor detects presence (sensor = '1') we reset the
            -- counter so the door stays open longer; likewise if the
            -- counter reaches MAX_CONTAGEM we reset (timer_expirou logic
            -- will be asserted for the combinational logic to observe).
            if proximo_estado = ABERTA then
                if sensor = '1' or contador = MAX_CONTAGEM then
                    contador <= 0;
                else
                    contador <= contador + 1;
                end if;
            else
                contador <= 0;
            end if;
        end if;
    end process;
    
    -- Combinational flag asserted when contador reached the programmed max.
    -- Used by the next-state logic to decide when to close the door after
    -- the configured open timeout.
    timer_expirou <= '1' when contador = MAX_CONTAGEM else '0';

    -- Combinational next-state logic. This block looks at the current
    -- state and inputs and decides the next state. It is purely
    -- combinational: outputs (proximo_estado) depend only on inputs.
    process(estado_atual, sensor, fechar_manual, fim_curso_aberta, fim_curso_fechada, timer_expirou)
    begin
        -- Default to remain in the same state unless a transition condition
        -- is met. This prevents inferred latches for proximo_estado.
        proximo_estado <= estado_atual;

        case estado_atual is
            when FECHADA =>
                -- From closed, start opening if the sensor is triggered
                if sensor = '1' then proximo_estado <= ABRINDO; end if;
            when ABRINDO =>
                -- While opening, wait for the end-of-travel switch that
                -- indicates the door is fully open.
                if fim_curso_aberta = '1' then proximo_estado <= ABERTA; end if;
            when ABERTA =>
                -- While open, close the door if either:
                --  - the open timeout expired AND the sensor no longer detects
                --    presence (timer_expirou = '1' and sensor = '0'), or
                --  - manual close command issued (fechar_manual = '1').
                if (timer_expirou = '1' and sensor = '0') or (fechar_manual = '1') then
                    proximo_estado <= FECHANDO;
                end if;
            when FECHANDO =>
                -- While closing, wait for the closed end-of-travel switch
                -- to confirm the door is fully closed.
                if fim_curso_fechada = '1' then proximo_estado <= FECHADA; end if;
        end case;
    end process;

    -- Output logic: drive motor signals according to the current state.
    -- This is a simple combinational block mapping states to motor outputs.
    process(estado_atual)
    begin
        motor_abrir  <= '0';
        motor_fechar <= '0';
        case estado_atual is
            when ABRINDO => motor_abrir  <= '1';
            when FECHANDO => motor_fechar <= '1';
            when others => null;
        end case;
    end process;
end architecture rtl;