# ============================================================================
#  VkWizardVS — CMake raiz
#  Resolve as dependências comuns (Vulkan, SDL3) e delega cada módulo para o
#  seu próprio CMakeLists em modules/<nome>/.
# ============================================================================

# Versão mínima do CMake. 3.28 já cobre tudo que usamos (FetchContent,
# cxx_std_26, FindVulkan moderno) e é mais portável do que exigir a 4.x.
cmake_minimum_required(VERSION 3.28)

# Nome do projeto e linguagem. Declarar só CXX evita procurar um compilador C à toa.
project(VkWizardVS LANGUAGES CXX)

# Padrão da linguagem para todos os alvos do projeto.
# Obs.: o ideal "moderno" seria target_compile_features(<alvo> PUBLIC cxx_std_26),
# mas este MSVC + CMake ainda não reconhece cxx_std_26 como feature, então usamos
# a forma global (que apenas passa /std:c++latest). Trocar quando houver suporte.
set(CMAKE_CXX_STANDARD 26)            # pede C++26
# CMAKE_CXX_STANDARD_REQUIRED deve ficar OFF: este CMake não conhece a flag exata
# de C++26 para o MSVC; com REQUIRED OFF ele "decai" para o /std:c++latest (o mais
# alto disponível) em vez de abortar. Ligar isto faz a configuração falhar.
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS OFF)         # usa /std:c++latest puro, sem extensões gnu

# ---- Vulkan ----------------------------------------------------------------
# Usa o SDK instalado (variável de ambiente VULKAN_SDK). Cria o alvo importado
# Vulkan::Vulkan com os headers e a lib do loader já configurados.
find_package(Vulkan REQUIRED)

# ---- SDL3 ------------------------------------------------------------------
# Baixa e compila a SDL3 junto do projeto — nada precisa estar instalado no PC.
# O código-fonte é clonado em build/_deps/ (fora do repositório).
include(FetchContent) # Como se fosse um include do C pra criar os comandos logo abaixo dele.
FetchContent_Declare(SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-3.4.10   # fixe uma tag p/ builds reprodutíveis (ajuste à vontade)
    GIT_SHALLOW    TRUE) # Só descreve a dependência. Não baixa nada
FetchContent_MakeAvailable(SDL3)  # Clona a tag e faz o add_subdirectory() dele, tornando os alvos
                # dele disponíveis.

# após esta linha existe o alvo SDL3::SDL3

# ---- GLM -------------------------------------------------------------------
# Biblioteca de matemática (vetores/matrizes) header-only. Baixada e disponível
# como o alvo glm::glm — não compila nada, só propaga os headers.
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.3            # fixe uma tag p/ builds reprodutíveis
    GIT_SHALLOW    TRUE)
# Não compila a lib do GLM: usamos só os headers. Sem isto, o GLM cria um alvo
# compilado que força CMAKE_CXX_STANDARD_REQUIRED=ON e exige CXX26 estrito —
# flag que o MSVC não reconhece, quebrando o configure. Com OFF, o alvo glm vira
# INTERFACE (header-only) e o problema do dialeto desaparece.
set(GLM_BUILD_LIBRARY OFF)
FetchContent_MakeAvailable(glm)

# Configurações do GLM exigidas pelo Vulkan, propagadas a TODO consumidor do alvo:
#   GLM_FORCE_DEPTH_ZERO_TO_ONE -> Vulkan usa profundidade 0..1 (não -1..1 do OpenGL)
#   GLM_FORCE_RADIANS           -> ângulos sempre em radianos, evita surpresas
# INTERFACE = aplica a quem linka glm::glm, não ao build (header-only) do próprio GLM.
target_compile_definitions(glm INTERFACE
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    GLM_FORCE_RADIANS)

# após esta linha existe o alvo glm::glm

# ---- Módulos (ordem = de baixo para cima na pilha de dependências) ---------
add_subdirectory(modules/core)     # tipos/utilitários base, sem dependências externas
add_subdirectory(modules/engine)   # renderer + plataforma (usa core + Vulkan + SDL3)
add_subdirectory(modules/game)     # executável final (usa engine)

# add_subdirectory é como se fosse uma chamada. as variáveis internas ficam no escopo interno.
