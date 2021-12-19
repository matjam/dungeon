//
// Created by matjam on 12/16/2021.
//

#include "Util.h"
#include <spdlog/spdlog.h>

std::string getEnv(std::string name) {
    char *env_var;
    size_t requiredSize;

    getenv_s(&requiredSize, NULL, 0, name.c_str());
    if (requiredSize == 0) {
        SPDLOG_DEBUG("environ variable {} doesn't exist", name);
        return "";
    }

    env_var = (char *) malloc(requiredSize * sizeof(char));
    if (!env_var) {
        SPDLOG_CRITICAL("failed malloc()");
        return "";
    }

    // Get the value of the LIB environment variable.
    getenv_s(&requiredSize, env_var, requiredSize, name.c_str());
    std::string value = std::string(env_var);
    free(env_var);

    return value;
}