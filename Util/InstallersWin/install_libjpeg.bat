@echo off
setlocal

rem BAT script that downloads and installs a ready to use
rem x64 libpng build for CARLA (carla.org).
rem Run it through a cmd with the x64 Visual C++ Toolset enabled.

set LOCAL_PATH=%~dp0
set FILE_N=    -[%~n0]:

rem Print batch params (debug purpose)
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

:arg-parse
if not "%1"=="" (
    if "%1"=="--build-dir" (
        set BUILD_DIR=%~dpn2
        shift
    )
    if "%1"=="--zlib-install-dir" (
        set ZLIB_INST_DIR=%~dpn2
        shift
    )
    if "%1"=="-h" (
        goto help
    )
    if "%1"=="--help" (
        goto help
    )
    shift
    goto :arg-parse
)

if "%ZLIB_INST_DIR%" == "" (
    echo %FILE_N% You must specify a zlib install directory using [--zlib-install-dir]
    goto bad_exit
)
if not "%ZLIB_INST_DIR:~-1%"=="\" set ZLIB_INST_DIR=%ZLIB_INST_DIR%\

rem If not set set the build dir to the current dir
if "%BUILD_DIR%" == "" set BUILD_DIR=%~dp0
if not "%BUILD_DIR:~-1%"=="\" set BUILD_DIR=%BUILD_DIR%\

rem ============================================================================
rem -- Local Variables ---------------------------------------------------------
rem ============================================================================

set LIBJPEG_BASENAME=libjpeg
set LIBJPEG_VERSION=2.0.90
set LIBJPEG_INSTALL_DIR=%BUILD_DIR%%LIBJPEG_BASENAME%\

if exist "%LIBJPEG_INSTALL_DIR%" (
    goto already_build
)

powershell -Command "Expand-Archive '%ZLIB_INST_DIR%/libjpeg.zip' -DestinationPath '%BUILD_DIR%'"

echo.
goto good_exit

:already_build
    echo %FILE_N% A libjpeg installation already exists.
    echo %FILE_N% Delete "%LIBJPEG_INSTALL_DIR%" if you want to force a rebuild.
    goto good_exit

:good_exit
    echo %FILE_N% Exiting...
    rem A return value used for checking for errors
    endlocal & set install_libjpeg=%LIBJPEG_INSTALL_DIR%
    exit /b 0
    