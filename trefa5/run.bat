@echo off
rem simulate a long-running process
ghdl -a flipflop_d.vhd
ghdl -a registrador_8bits.vhdl
ghdl -a tb_registrador_8bits.vhdl

ghdl -r tb_registrador_8bits --vcd=tb_registrador_8bits.vcd 
gtkwave tb_registrador_8bits.vcd

del *.cf
pause