# !/bin/sh

GREEN="\e[1;32m"
RED="\e[0;31m"
PURPLE="\e[0;35m"
NC="\e[0m"

# Remove existing .spv files
# spvs=$(find . -name '*.spv')
# for spv in $spvs; do
#   rm "$spv"
# done

function compile {
  local file_path="$1"

  local dir=$(dirname "$file_path")
  local filename=$(basename "$file_path")
  local basename=${filename%.*}

  slangc $file_path          \
    -target spirv            \
    -profile spirv_1_4       \
    -emit-spirv-directly     \
    -fvk-use-entrypoint-name \
    -entry vertMain          \
    -entry fragMain          \
    -o "$dir/${basename}.spv"

  if [ $? -eq 0 ]; then
    echo -e "${GREEN}[SUCCESS]${NC} $file_path -> ${PURPLE}slang_${basename}.spv${NC}"
  else
    echo -e "${RED}[FAILED]${NC} $file_path${NC}"
  fi
}

# Compile .slang files to .spv
echo -e "${PURPLE}SLANGC: compiling...${NC}"
shaders=$(find . -name '*.slang')
for shader in $shaders; do
  compile $shader
done
