@echo off
rem This is and auto-restart script for the eAthena Ragnarok Online Server Emulator.
rem It will also keep the map server OPEN after it crashes to that errors may be
rem more easily identified
rem Writen by Jbain
echo Jbain's eAthena Start script for Windoze
start cmd /k login-start.bat
start cmd /k char-start.bat
start cmd /k map-start.bat