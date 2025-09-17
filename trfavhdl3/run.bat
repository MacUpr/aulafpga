@echo off
rem simulate a long-running process
ghdl -a tr.vhd
ghdl -a tb_tr.vhd

ghdl -e tb_classificador_light
ghdl -r tb_classificador_light --vcd=tb_tr.vcd 
gtkwave tb_tr.vcd
del *.cf
pause