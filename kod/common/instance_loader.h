#ifndef PEA2_INSTANCE_LOADER_H
#define PEA2_INSTANCE_LOADER_H
#pragma once

#include <string>
#include "tsp_instance.h"

TSPInstance loadInstance(const std::string& filePath);
#endif //PEA2_INSTANCE_LOADER_H
