#include "hwloc_memory.hpp"
#include "../logger/logger.hpp"
#include <hwloc.h>
#include <iostream>

bool set_thisthread_membind(hwloc_topology_t _topology) {
	return hwloc_topology_get_support(_topology)->membind->set_thisthread_membind;
}

int processing_units_depth(hwloc_topology_t _topology) {
	return hwloc_get_type_depth(_topology, HWLOC_OBJ_PU);
}

int width_of_processing_unit(hwloc_topology_t _topology, int depth) {
	return hwloc_get_nbobjs_by_depth(_topology, depth);
}

hwloc_obj_t get_processing_unit_pointer(hwloc_topology_t _topology, int depth, int index, size_t core_id) {
	hwloc_obj_t ptr = hwloc_get_obj_by_depth(_topology, depth, index);
	if (ptr->os_index == core_id) {
		return ptr;
	}
	return NULL;
}

int pin_thread_to_processing_unit(hwloc_topology_t _topology, hwloc_obj_t ptr) {
	return hwloc_set_membind_nodeset(_topology, ptr->nodeset, HWLOC_MEMBIND_BIND, HWLOC_MEMBIND_THREAD);
}

/** pin memory to NUMA node
 *
 * Set the default memory policy for the current thread to bind memory to the
 * NUMA node.
 *
 * @param puId core id
 */
bool bindMemoryToNUMANode( std::size_t puId )
{
	hwloc_topology_t _topology;
	hwloc_topology_init(&_topology);
	hwloc_topology_load(_topology);

	if(!set_thisthread_membind(_topology)) {
		logger::log_error(__FILE__, __LINE__, "Binding the current thread only is NOT supported.");
		hwloc_topology_destroy(_topology);
		return false;
	}

	const int depth = processing_units_depth(_topology);
	const int processing_unit_width = width_of_processing_unit(_topology, depth);
	for( size_t i = 0; i < processing_unit_width; i++ ) {
		hwloc_obj_t pu = get_processing_unit_pointer(_topology, depth, i, puId);
		if (pu == NULL) {
			continue;
		}
		const int pinned = pin_thread_to_processing_unit(_topology, pu);
		if(0 > pinned) {
			logger::log_error(__FILE__, __LINE__, "Binding the current thread failed.");
		} else {
			logger::log_error(__FILE__, __LINE__, "Binding the current thread succeeded.");
			hwloc_topology_destroy(_topology);
			return true;
		}
	}

	hwloc_topology_destroy(_topology);
	return false;
}
