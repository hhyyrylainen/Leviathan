::Moves a .sym file to the right sub directory
setlocal ENABLEDELAYEDEXPANSION

set symbol_file=%1
set target_name=%2

set /p file_info=< %symbol_file%

set s=%file_info%
set t=%s%

set /a counter=0

:loop
for /f "tokens=1*" %%a in ("%t%") do (
    
   set looped=%%a
   
   set /a counter=counter+1
   set t=%%b
   set dir_name=%looped%
   
   if "%looped%" equ "MODULE" set /a counter=1
   
   if !counter! equ 4 goto :endloop

   )
if defined t goto :loop
:endloop

set dest_dir=%target_name%/%dir_name%

echo "Moving %symbol_file% to subfolder %dir_name% target is %dest_dir%"

if not exist "%target_name%" mkdir "%target_name%"
if not exist "%dest_dir%" mkdir "%dest_dir%"
move "%symbol_file%" "%dest_dir%"
echo "%symbol_file% -> %dest_dir%/%symbol_file%"
endlocal

