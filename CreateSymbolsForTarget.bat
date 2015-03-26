::Dumps symbols of a target and then moves them
::This should be executed in the Symbols directory
SET filename=%~n1

::Create the symbol file
dump_syms.exe "%~p1%filename%.pdb" > "%filename%.sym"

FOR /F "usebackq" %%A IN ('%filename%.sym') DO set size=%%~zA

if %size% gtr 0 (
    echo Created symbol file %filename%.sym of size %size%
) else (
    echo Failed to create symbol file, Did you forget to build RelWithDebInfo configuration?
    exit 1
)



::Move it to the right place
call MoveSymbolFile.bat %filename%.sym %filename%

exit 0
