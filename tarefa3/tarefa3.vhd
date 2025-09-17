library ieee;
use ieee.std_logic_1164.all;

entity tarefa3 is
  port ( A, B, C, D : in std_logic;
         s    : out std_logic );
end tarefa3;

architecture rtl of tarefa3 is
begin
    s <= (A and B)or(A and C)or(B and C and D); -- A B	+	A C	+	B C D
end rtl;
