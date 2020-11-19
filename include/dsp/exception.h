#pragma once

#include <stdexcept>
#include <system_error>

namespace dsp {

class dsp_error: public std::runtime_error
{
public:
    dsp_error(const std::string& what_arg): std::runtime_error(what_arg) {}
private:
    std::error_code errc_;
};

class pipeline_error: public dsp_error
{
public:
    pipeline_error(const std::string& what_arg): dsp_error(what_arg) {}
};

class filter_error: public dsp_error
{
public:
    filter_error(const std::string& what_arg): dsp_error(what_arg) {}
};

class link_error: public dsp_error
{
public:
    link_error(const std::string& what_arg): dsp_error(what_arg) {}
};

} /* namespace dsp */
