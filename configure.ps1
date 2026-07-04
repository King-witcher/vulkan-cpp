# -S = Source
# -B = build directory
# Gerador: "Ninja Multi-Config" gera Debug e Release no mesmo build/, escolhidos
# na hora do build com --config — é o que os tasks do VSCode usam. Vai via env
# var CMAKE_GENERATOR, que equivale ao -G.
#
# O configure precisa do ambiente do MSVC: o CMake compila um programa de teste
# pra validar o compilador, e tanto o clang++ (target msvc) quanto o cl.exe
# (usado no C da SDL) leem INCLUDE/LIB do ambiente. O Launch-VsDevShell é o
# "vcvars64 do PowerShell" — carrega esse ambiente na sessão atual.
#   -Arch/-HostArch amd64      -> sem isso o default é x86 (tools de 32 bits!)
#   -SkipAutomaticLocation     -> não faz cd pra pasta do Visual Studio
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Launch-VsDevShell.ps1' -Arch amd64 -HostArch amd64 -SkipAutomaticLocation | Out-Null

$env:CMAKE_GENERATOR = 'Ninja Multi-Config'
cmake -S . -B build
