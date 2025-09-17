@echo off

yosys -p "read_verilog blink.v; synth_ecp5 -json blink.json -abc9"

nextpnr-ecp5 --json blink.json --lpf blink.lpf --textcfg blink.config  --package CABGA381 --45k --speed 6

ecppack --input blink.config --bit blink.bit

openFPGALoader -b colorlight-i9 blink.bit