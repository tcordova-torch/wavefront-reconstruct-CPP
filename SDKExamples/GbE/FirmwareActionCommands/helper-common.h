// ============================================================================
// === Common helper functions
// ===
// === (c) 2009-2016 Silicon Software GmbH
// ============================================================================

#pragma once

#include <sstream>

void readCommandLineArgument(const char* arg, unsigned int* var) {
    std::istringstream iss(arg);
    iss >> *var;
}

std::string getDmaString(unsigned int dma) {
    std::ostringstream oss;
    oss << "DMA " << dma;
    return oss.str();
}
