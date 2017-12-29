//
// Created by Snow Flake on 12/29/17.
//

#ifndef MONERO_CPU_MINER_LOGGER_H
#define MONERO_CPU_MINER_LOGGER_H

#include <string>

namespace logger {
	void log_success(const char * file, const int line, std::string message);
	void log_error(const char * file, const int line, std::string message);
}


#endif //MONERO_CPU_MINER_LOGGER_H
