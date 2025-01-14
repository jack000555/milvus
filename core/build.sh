#!/bin/bash

BUILD_OUTPUT_DIR="cmake_build"
BUILD_TYPE="Debug"
BUILD_UNITTEST="OFF"
INSTALL_PREFIX=$(pwd)/milvus
MAKE_CLEAN="OFF"
BUILD_COVERAGE="OFF"
DB_PATH="/tmp/milvus"
PROFILING="OFF"
USE_JFROG_CACHE="OFF"
RUN_CPPLINT="OFF"
CUSTOMIZATION="OFF" # default use ori faiss
CUDA_COMPILER=/usr/local/cuda/bin/nvcc

CUSTOMIZED_FAISS_URL="${FAISS_URL:-NONE}"
wget -q --method HEAD ${CUSTOMIZED_FAISS_URL}
if [ $? -eq 0 ]; then
  CUSTOMIZATION="ON"
else
  CUSTOMIZATION="OFF"
fi

while getopts "p:d:t:ulrcgjhx" arg
do
        case $arg in
             p)
                INSTALL_PREFIX=$OPTARG
                ;;
             d)
                DB_PATH=$OPTARG
                ;;
             t)
                BUILD_TYPE=$OPTARG # BUILD_TYPE
                ;;
             u)
                echo "Build and run unittest cases" ;
                BUILD_UNITTEST="ON";
                ;;
             l)
                RUN_CPPLINT="ON"
                ;;
             r)
                if [[ -d ${BUILD_OUTPUT_DIR} ]]; then
                    rm ./${BUILD_OUTPUT_DIR} -r
                    MAKE_CLEAN="ON"
                fi
                ;;
             c)
                BUILD_COVERAGE="ON"
                ;;
             g)
                PROFILING="ON"
                ;;
             j)
                USE_JFROG_CACHE="ON"
                ;;
             x)
                CUSTOMIZATION="OFF" # force use ori faiss
                ;;
             h) # help
                echo "

parameter:
-p: install prefix(default: $(pwd)/milvus)
-d: db data path(default: /tmp/milvus)
-t: build type(default: Debug)
-u: building unit test options(default: OFF)
-l: run cpplint, clang-format and clang-tidy(default: OFF)
-r: remove previous build directory(default: OFF)
-c: code coverage(default: OFF)
-g: profiling(default: OFF)
-j: use jfrog cache build directory(default: OFF)
-h: help

usage:
./build.sh -p \${INSTALL_PREFIX} -t \${BUILD_TYPE} [-u] [-l] [-r] [-c] [-g] [-j] [-h]
                "
                exit 0
                ;;
             ?)
                echo "ERROR! unknown argument"
        exit 1
        ;;
        esac
done

if [[ ! -d ${BUILD_OUTPUT_DIR} ]]; then
    mkdir ${BUILD_OUTPUT_DIR}
fi

cd ${BUILD_OUTPUT_DIR}

CMAKE_CMD="cmake \
-DBUILD_UNIT_TEST=${BUILD_UNITTEST} \
-DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX}
-DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
-DCMAKE_CUDA_COMPILER=${CUDA_COMPILER} \
-DBUILD_COVERAGE=${BUILD_COVERAGE} \
-DMILVUS_DB_PATH=${DB_PATH} \
-DMILVUS_ENABLE_PROFILING=${PROFILING} \
-DUSE_JFROG_CACHE=${USE_JFROG_CACHE} \
-DCUSTOMIZATION=${CUSTOMIZATION} \
-DFAISS_URL=${CUSTOMIZED_FAISS_URL} \
../"
echo ${CMAKE_CMD}
${CMAKE_CMD}

if [[ ${MAKE_CLEAN} == "ON" ]]; then
    make clean
fi

if [[ ${RUN_CPPLINT} == "ON" ]]; then
    # cpplint check
    make lint
    if [ $? -ne 0 ]; then
        echo "ERROR! cpplint check failed"
        rm -f CMakeCache.txt
        exit 1
    fi
    echo "cpplint check passed!"

    # clang-format check
    make check-clang-format
    if [ $? -ne 0 ]; then
        echo "ERROR! clang-format check failed"
        rm -f CMakeCache.txt
        exit 1
    fi
    echo "clang-format check passed!"

#    # clang-tidy check
#    make check-clang-tidy
#    if [ $? -ne 0 ]; then
#        echo "ERROR! clang-tidy check failed"
#        rm -f CMakeCache.txt
#        exit 1
#    fi
#    echo "clang-tidy check passed!"

    rm -f CMakeCache.txt
else
    # compile and build
    make -j 4 || exit 1

    # strip binary symbol
    if [[ ${BUILD_TYPE} != "Debug" ]]; then
        strip src/milvus_server
    fi

    make install || exit 1
fi