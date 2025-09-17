@echo off
echo ========================================
echo Compilando os arquivos VHDL com GHDL...
echo ========================================
ghdl -a updown_counter.vhd
ghdl -a tb_updown_counter.vhd

echo ========================================
echo Elaborando o testbench...
echo ========================================
ghdl -e tb_updown_counter

echo ========================================
echo Executando simulação...
echo ========================================
ghdl -r tb_updown_counter --vcd=updown_counter.vcd --stop-time=200ns

echo ========================================
echo Abrindo GTKWave...
echo ========================================
gtkwave updown_counter.vcd

del *.cf
pause