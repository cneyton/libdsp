#pragma once

#include <stdexcept>
#include <system_error>

namespace dsp {

enum class Errc {
    invalid_chunk_format = 1,
    link_forbidden,
    pad_occupied,
    pad_unknown,
    format_negotiation_failed,
    invalid_parameters,
    duplicate_filter,
};

struct ErrorCategory: public std::error_category
{
    const char * name() const noexcept override {return "dsp";};
    std::string message(int e) const override;
};

std::error_code make_error_code(Errc e);

class dsp_error: public std::runtime_error
{
public:
    dsp_error(Errc e): dsp_error(make_error_code(e)) {};
    const std::error_code& code() const noexcept {return errc_;};
private:
    dsp_error(const std::error_code& e): std::runtime_error(e.message()), errc_(e) {}
    std::error_code errc_;
};

} /* namespace dsp */

namespace std {

template <>
struct is_error_code_enum<dsp::Errc> : true_type {};

} /* namespace std */
