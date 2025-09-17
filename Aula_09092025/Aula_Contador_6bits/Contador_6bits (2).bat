@echo off

cd /d impl1

openFPGALoader -b colorlight-i9 --unprotect-flash -f --verify Contador_6bits_impl1.bit