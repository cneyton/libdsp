#ifndef FFT_FILTER_H
#define FFT_FILTER_H

#include "common/log.h"
#include "sigpack.h"
#include "filter.h"
#include "chunk.h"
#include "link.h"

namespace filter
{

template<typename T>
class FFT: public Filter
{
public:
    FFT(common::Logger logger, uint N): Log(logger), Filter(logger), fftw_(sp::FFTW(N, FFTW_MEASURE))
    {
        // Create the plan by first application of fft
        arma::cx_vec vec(N);
        fftw_.fft_cx(vec, vec);
    }
    virtual ~FFT() {};

    virtual int activate()
    {
        log_debug(logger_, "fft filter {} activated", this->name_);
        auto in_chunk = std::make_shared<Chunk>(logger_);
        if (!inputs_.at(0)->pop(in_chunk)) {
            ready_ = false;
            return 0;
        }
        auto size = arma::size(*in_chunk);
        auto out_chunk = std::make_shared<Chunk>(*in_chunk);

        for (uint k = 0; k < size.n_slices; k++) {
            for (uint j = 0; j < size.n_cols; j++) {
                auto ptr  = out_chunk->slice_colptr(k, j);
                arma::Col<T> in(ptr, size.n_rows, false, true) ;
                fftw_.fft_cx(in, in);
            }
        }

        if (verbose_) {
            in_chunk->print();
            out_chunk->print();
        }

        outputs_.at(0)->push(out_chunk);
        return 0;
    }

private:
    sp::FFTW fftw_;
};

} /* namespace filter */

#endif /* FFT_FILTER_H */
