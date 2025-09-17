@echo off
echo ========================================
echo Compilando os arquivos VHDL com GHDL...
echo ========================================
ghdl -a piso.vhd
ghdl -a tb_piso_shift.vhd

echo ========================================
echo Elaborando o testbench...
echo ========================================
ghdl -e tb_piso_shift

echo ========================================
echo Executando simulação...
echo ========================================
ghdl -r tb_piso_shift --vcd=piso_shift.vcd

echo ========================================
echo Abrindo GTKWave...
echo ========================================
gtkwave piso_shift.vcd


del *.cf
pause