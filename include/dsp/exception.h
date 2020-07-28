#pragma once

#include <stdexcept>

namespace dsp
{

class pipeline_error: public std::runtime_error
{
public:
    pipeline_error(const std::string& what_arg): std::runtime_error(what_arg) {}
};

class filter_error: public std::runtime_error
{
public:
    filter_error(const std::string& what_arg): std::runtime_error(what_arg) {}
};

} /* namespace dsp */
