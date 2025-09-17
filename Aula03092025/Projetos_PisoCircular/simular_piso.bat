@echo off
echo ========================================
echo Compilando registrador PISO circular...
echo ========================================
ghdl -a piso.vhd
ghdl -a tb_piso_shift.vhd
ghdl -e tb_piso_shift

echo ========================================
echo Executando simulação e gerando VCD...
echo ========================================
ghdl -r tb_piso_shift --vcd=piso.vcd

echo ========================================
echo Abrindo no GTKWave...
echo ========================================
gtkwave piso.vcd

del *.cf
pause