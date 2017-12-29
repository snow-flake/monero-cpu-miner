#ifndef C_BLAKE_DO_HASH_H
#define C_BLAKE_DO_HASH_H

#include <cstring>

void do_blake_hash(const void* input, std::size_t len, char* output);

#endif //C_BLAKE_DO_HASH_H