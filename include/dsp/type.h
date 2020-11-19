#pragma once

namespace dsp {

template<typename T>
struct IQ
{
    using elem_type = T;
    T i; T q;
};

} /* namespace dsp */
