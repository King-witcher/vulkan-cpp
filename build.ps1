# Compila com os arquivos gerados em build/.
# Config padrão: Debug. Pra outra config: ./build.ps1 Release
param([string]$Config = 'Debug')

# O ambiente do MSVC é necessário no build também: o Ninja chama cl.exe (C da
# SDL) e clang++/lld direto, e eles leem INCLUDE/LIB do ambiente.
# (Detalhes das flags no configure.ps1.)
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Launch-VsDevShell.ps1' -Arch amd64 -HostArch amd64 -SkipAutomaticLocation | Out-Null

cmake --build build --config $Config
