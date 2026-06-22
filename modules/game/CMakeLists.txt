# ----------------------------------------------------------------------------
#  Módulo: game  —  o executável final (ponto de entrada Main.cpp)
# ----------------------------------------------------------------------------

# Nome mantido como VkWizardVS para casar com o .vscode/launch.json
# (build/<Config>/VkWizardVS.exe). Como NÃO sobrescrevemos o diretório de saída,
# o gerador do Visual Studio coloca o .exe em build/Debug ou build/Release.
add_executable(VkWizardVS src/Main.cpp)

# Só precisa do engine; Vulkan, SDL3 e core entram de carona (são PUBLIC no engine).
target_link_libraries(VkWizardVS PRIVATE VkWizard::engine)

# Sem isto, o gerador do Visual Studio colocaria o .exe em build/modules/game/<Config>/.
# Forçamos a saída para build/<Config>/ (o gerador acrescenta Debug/Release sozinho),
# que é exatamente o caminho que o .vscode/launch.json espera.
set_target_properties(VkWizardVS PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# A SDL3 é compilada como DLL pelo FetchContent. Copia o SDL3.dll para o lado
# do .exe depois de cada build — caso contrário o programa não acha a DLL ao rodar.
add_custom_command(TARGET VkWizardVS POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3> $<TARGET_FILE_DIR:VkWizardVS>
    VERBATIM)
