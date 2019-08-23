// ============================================================================
// === Gbe error handling functions
// ===
// === (c) 2009-2016 Silicon Software GmbH
// ============================================================================

#pragma once

#include <gbe.h>

#include <stdexcept>
#include <sstream>

void throwGbeError(int errcode, const char* func) {
    std::ostringstream oss;
    oss << func << " failed: " << Gbe_getErrorDescription(errcode);
    throw std::runtime_error(oss.str());
}
