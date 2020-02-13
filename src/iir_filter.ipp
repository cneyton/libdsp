#pragma once

#include "chunk.h"
#include "link.h"
#include <memory>

namespace filter
{

template<typename T>
IIR<T>::IIR(common::Logger logger): Log(logger), Filter(logger)
{
}

template<typename T>
IIR<T>::IIR(common::Logger logger, uint16_t nb_filters): Log(logger), Filter(logger),
    filters_(std::vector<sp::IIR_filt<T, double, T>>(nb_filters))
{
    std::for_each(filters_.begin(), filters_.end(), [&](auto& f){f.clear();});
}

template<typename T>
IIR<T>::IIR(common::Logger logger, uint16_t nb_filters, const arma::vec& b, const arma::vec& a):
    IIR<T>(logger, nb_filters)
{
    std::for_each(filters_.begin(), filters_.end(), [&](auto& f){f.set_coeffs(b, a);});
}

template<typename T>
IIR<T>::~IIR()
{
}

template<typename T>
int IIR<T>::activate()
{
    log_debug(logger_, "iir filter {} activated", this->name_);

    auto in_chunk = std::make_shared<Chunk>(logger_);
    if (!inputs_.at(0)->pop(in_chunk)) {
        ready_ = false;
        return 0;
    }
    auto size = arma::size(*in_chunk);
    auto out_chunk = std::make_shared<Chunk>(logger_, size);
    out_chunk->zeros();
    uint n = 0;
    for (uint k = 0; k < size.n_slices; k++) {
        for (uint j = 0; j < size.n_cols; j++) {
            auto in_ptr  = in_chunk->slice_colptr(k, j);
            auto out_ptr = out_chunk->slice_colptr(k, j);
            arma::Col<T> in(in_ptr, size.n_rows, false, true) ;
            arma::Col<T> out(out_ptr, size.n_rows, false, true) ;
            out = filters_[n].filter(in);
        }
    }

    if (verbose_) {
        in_chunk->print();
        out_chunk->print();
    }

    outputs_.at(0)->push(out_chunk);
    return 0;
}

} /* namespace filter */
