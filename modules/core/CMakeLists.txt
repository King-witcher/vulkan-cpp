# ----------------------------------------------------------------------------
#  Módulo: core  —  tipos e utilitários base (RustTypes.h, Panic.h)
#  É header-only (não há .cpp), então vira uma biblioteca INTERFACE: não gera
#  binário algum, apenas propaga headers e requisitos para quem a consome.
# ----------------------------------------------------------------------------
add_library(core INTERFACE)
add_library(VkWizard::core ALIAS core)   # nome com namespace = padrão moderno de consumo

# Expõe a pasta include/ a quem linkar o core (INTERFACE = só para consumidores).
# Mantém os #include "RustTypes.h" / "Panic.h" existentes resolvendo, sem alterar o código.
target_include_directories(core INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

# O padrão C++26 vem da configuração global no CMakeLists raiz (CMAKE_CXX_STANDARD),
# aplicada aos alvos que de fato compilam (engine e game).
