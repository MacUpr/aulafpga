@echo off

cd /d impl1

openFPGALoader -b colorlight-i9 --unprotect-flash -f --verify blink_impl1.bit