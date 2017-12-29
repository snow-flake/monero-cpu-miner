cmake_minimum_required(VERSION 3.8)
project(monero_cpu_miner)

set(CMAKE_CXX_STANDARD 11)

include_directories(/usr/local/Cellar/hwloc/1.11.8/include)
link_directories(/usr/local/Cellar/hwloc/1.11.8/lib)
link_libraries(hwloc)

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
)

add_executable(monero_cpu_miner ${SOURCE_FILES})
