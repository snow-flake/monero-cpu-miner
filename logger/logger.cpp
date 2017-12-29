//
// Created by Snow Flake on 12/29/17.
//

#include "logger.hpp"
#include <json.hpp>
#include <iostream>

namespace logger {
	void log_success(const char * file, const int line, std::string message) {
		nlohmann::json json;
		json["level"] = "success";
		json["file"] = file;
		json["line"] = line;
		json["message"] = message;
		std::cout << json << std::endl;
	}

	void log_error(const char * file, const int line, std::string message) {
		nlohmann::json json;
		json["level"] = "error";
		json["file"] = file;
		json["line"] = line;
		json["message"] = message;
		std::cout << json << std::endl;
	}
}

