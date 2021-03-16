#pragma once

#include <string>
#include <typeinfo>

#include "format.h"

namespace dsp {

struct Pad
{
    std::string    name;
    Format         format;
};

} /* namespace dsp */
