#include "dsp/dsp_error.h"

namespace dsp {

const ErrorCategory dsp_error_category {};

std::string ErrorCategory::message(int e) const
{
    switch (static_cast<Errc>(e)) {
    case Errc::invalid_chunk_format:      return "invalid chunk format";
    case Errc::link_forbidden:            return "linking forbidden";
    case Errc::pad_occupied:              return "pad occupied";
    case Errc::pad_unknown:               return "pad unknown";
    case Errc::format_negotiation_failed: return "format negotation failed";
    default:                              return "unknown error code";
    }
}

std::error_code make_error_code(Errc e)
{
    return {static_cast<int>(e), dsp_error_category};
}

} /* namespace dsp */
