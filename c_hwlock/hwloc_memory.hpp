#ifndef MONERO_CPU_MINER_C_HWLOCK_HWLOCK_MEMORY_H
#define MONERO_CPU_MINER_C_HWLOCK_HWLOCK_MEMORY_H

#include <cstring>

/** pin memory to NUMA node
 *
 * Set the default memory policy for the current thread to bind memory to the
 * NUMA node.
 *
 * @param puId core id
 */
bool bindMemoryToNUMANode( std::size_t puId );

#endif //MONERO_CPU_MINER_C_HWLOCK_HWLOCK_MEMORY_H