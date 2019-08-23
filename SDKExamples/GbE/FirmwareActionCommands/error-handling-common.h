// ============================================================================
// === Common error handling functions
// ===
// === (c) 2009-2016 Silicon Software GmbH
// ============================================================================

#pragma once

#include <fgrab_prototyp.h>

#include <stdexcept>
#include <sstream>

void throwFgError(Fg_Struct* fg, const char* func) {
    std::ostringstream oss;
    oss << func << " failed: " << Fg_getLastErrorDescription(fg);
    throw std::runtime_error(oss.str());
}

void throwFgError(Fg_Struct* fg, int errcode, const char* func) {
    std::ostringstream oss;
    oss << func << " failed: " << Fg_getErrorDescription(fg, errcode);
    throw std::runtime_error(oss.str());
}

void throwError(const char* msg) {
    throw std::runtime_error(msg);
}
