attrib -r %~dp0.vs\*.* /s
attrib -r %~dp0Intermediate\*.* /s
attrib -r %~dp0Saved\*.* /s
attrib -r %~dp0Binaries\*.* /s
attrib -r %~dp0Plugins\*.* /s
@echo off
echo Eliminado atributo de solo lectura correctamente
PAUSE