@echo off
setlocal

rem Set the file name
set "FILENAME=bank.cpp"

rem Set the exported file name
set "EXPORT=bank.exe"

rem Set the C++ compiler version
set "VERSION=-std=c++20"

rem Set the compiler flags
set "CFLAGS=-Wall"

rem Log file for warnings and errors
set "LOGFILE=compile.log"

rem Path to the U.M.L (Unified Model Language)
set "UMLPATH=UML\html"

rem ASCII art using figlet
echo "-------------------------------------------"
echo "                  _         __ _ _         "
echo "                  | |       / _(_) |       "
echo "   _ __ ___   __ _| | _____| |_ _| | ___   "
echo "  | '_ ` _ \ / _` | |/ / _ \  _| | |/ _ \  "
echo "  | | | | | | (_| |   <  __/ | | | |  __/  "
echo "  |_| |_| |_|\__,_|_|\_\___|_| |_|_|\___|  "
echo "                                           "
echo "-------------------------------------------"
echo Commands: make, clean, run, uml, exit.

:loop

rem Get the user's choice
set /p choice=">> "

if "%choice%" == "make" (
    rem Compile the source file and log the output
    echo Compiling...
    g++ %CFLAGS% %VERSION% "%FILENAME%" -o "%EXPORT%" -lssl -lcrypto > "%LOGFILE%" 2>&1
    echo "g++ %CFLAGS% %VERSION% "%FILENAME%" -o "%EXPORT%" -lssl -lcrypto > "%LOGFILE%" 2>&1"
    if errorlevel 1 (
        echo Compilation failed. Check %LOGFILE% for details.
    ) else (
        echo Compilation successful.
    )
) else if "%choice%" == "clean" (
    echo Cleaning up...
    if exist "%EXPORT%" (
        del /q "%EXPORT%"
        echo del /q "%EXPORT%"
    )
    if exist "%LOGFILE%" (
        del /q "%LOGFILE%"
        echo del /a "%LOGFILE%"
    )
    echo Cleanup complete.
) else if "%choice%" == "run" (
    rem Run the executable if it exists
    if exist "%EXPORT%" (
        echo Running %EXPORT%...
        "%EXPORT%"
    ) else (
        echo %EXPORT% not found. Please compile first.
    )
) else if "%choice%" == "exit" (
    echo Exiting...
    exit /b 0
) else if "%choice%" == "cls" (
    cls
) else if "%choice%" == "uml" (
    rem Check if the HTML file exists before opening
    if exist "%UMLPATH%\annotated.html" (
        echo Opening UML HTML file...
        cd /d "%UMLPATH%"
        start "" "annotated.html"
        cd ..\..
    ) else (
        echo annotated.html not found in %UMLPATH%. Please check the path.
    )
) else (
    echo Invalid choice. Please try again.
)

goto loop
