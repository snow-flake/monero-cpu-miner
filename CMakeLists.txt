cmake_minimum_required(VERSION 3.8)
project(monero_cpu_miner)

# enforce C++11
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 11)

# activate sse2 and aes-ni
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2 -maes")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse2 -maes")
# activate static libgcc and libstdc++ linking
set(BUILD_SHARED_LIBRARIES OFF)
set(DL_LIB ${CMAKE_DL_LIBS})
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(LIBS "-static-libgcc -static-libstdc++ ${LIBS}")

include_directories(/usr/local/Cellar/cryptopp/5.6.5/include)
link_directories(/usr/local/Cellar/cryptopp/5.6.5/lib)

include_directories(/usr/local/Cellar/hwloc/1.11.8/include)
link_directories(/usr/local/Cellar/hwloc/1.11.8/lib)
link_libraries(hwloc cryptopp)

include_directories(includes)

set(
        SOURCE_FILES
        main.cpp
        logger/logger.hpp
        logger/logger.cpp
        c_blake/do_blake_hash.hpp
        c_blake/do_blake_hash.cpp
        c_blake/c_blake256.h
        c_blake/c_blake256.cpp
        c_groestl/do_groestl_hash.hpp
        c_groestl/do_groestl_hash.cpp
        c_groestl/c_groestl.hpp
        c_groestl/c_groestl.cpp
        c_groestl/groestl_tables.h
        c_jh/do_jh_hash.hpp
        c_jh/do_jh_hash.cpp
        c_jh/c_jh.hpp
        c_jh/c_jh.cpp
        c_keccak/do_keccak.hpp
        c_keccak/do_keccak.cpp
        c_keccak/c_keccak.hpp
        c_keccak/c_keccak.cpp
        c_skein/do_skein_hash.hpp
        c_skein/do_skein_hash.cpp
        c_skein/c_skein.hpp
        c_skein/c_skein.cpp
        c_skein/skein_port.hpp
        c_cryptonight/do_cryptonight.hpp
        c_cryptonight/do_cryptonight.cpp
        c_cryptonight/cryptonight.hpp
        c_cryptonight/cryptonight.cpp
        c_cryptonight/cryptonight_aesni.hpp
        c_cryptonight/int-util.hpp
        c_cryptonight/soft_aes.hpp
        c_cryptonight/cryptonight_aesni.cpp
        c_hwlock/hwloc_memory.hpp
        c_hwlock/hwloc_memory.cpp
        hex/hex_conversions.h
        hex/hex_conversions.cpp
)

add_executable(monero_cpu_miner ${SOURCE_FILES})
target_link_libraries(monero_cpu_miner hwloc cryptopp)
