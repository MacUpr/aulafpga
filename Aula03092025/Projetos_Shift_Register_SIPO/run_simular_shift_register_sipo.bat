@echo off
echo ========================================
echo Compilando os arquivos VHDL com GHDL...
echo ========================================
ghdl -a shift_register_sipo.vhd
ghdl -a tb_shift_sipo_2.vhd

echo ========================================
echo Elaborando o testbench...
echo ========================================
ghdl -e tb_shift_sipo_2

echo ========================================
echo Executando simulação...
echo ========================================
ghdl -r tb_shift_sipo_2 --vcd=shift_sipo.vcd --stop-time=200ns

echo ========================================
echo Abrindo GTKWave...
echo ========================================
gtkwave shift_sipo.vcd


del *.cf
pause