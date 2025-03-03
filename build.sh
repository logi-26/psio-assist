#!/bin/bash

# Verifica se estamos no diretório build
if [[ $(basename "$PWD") != "build" ]]; then
    echo "Criando diretório de build..."
    mkdir -p build
    
    echo "Entrando no diretório de build..."
    cd build
    
    echo "Configurando CMake pela primeira vez..."
    cmake ..
else
    echo "Já estamos no diretório build, apenas recompilando..."
fi

echo "Compilando..."
cmake --build . -j$(nproc)

if [ $? -eq 0 ]; then
    echo "Build realizado com sucesso!"
    echo "Executável está em build/psio-assist"
else
    echo "Erro durante a build!"
    exit 1
fi 
build.sh