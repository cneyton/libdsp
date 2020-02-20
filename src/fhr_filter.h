#ifndef FHR_FILTER_H
#define FHR_FILTER_H

#include "chunk.h"
#include "link.h"
#include "filter.h"
#include "correlate.h"
#include "peaks.h"
#include "interp.h"

namespace filter
{

template<typename T1 = double, typename T2 = double, typename T3 = T2>
class fhr: public Filter
{
public:
    fhr(common::Logger logger): Log(logger), Filter(logger) {}
    virtual ~fhr() {}

    struct period_range
    {
        T3 min;
        T3 max;
    };

    virtual int activate()
    {
        log_debug(logger_, "fhr filter {} activated", this->name_);

        auto in_chunk = std::make_shared<Chunk<T1>>(logger_);
        auto input    = dynamic_cast<Link<T1>*>(inputs_.at(0));
        if (!input->pop(in_chunk)) {
            ready_ = false;
            return 0;
        }

        auto in_size = arma::size(*in_chunk);

        arma::SizeCube out_size(in_size.n_cols, in_size.n_slices, 1);
        auto out0_chunk = std::make_shared<Chunk<T2>>(logger_, out_size);
        auto out1_chunk = std::make_shared<Chunk<T3>>(logger_, out_size);

        for (uint k = 0; k < in_size.n_slices; k++) {
            for (uint j = 0; j < in_size.n_cols; j++) {
                auto in_ptr  = in_chunk->slice_colptr(k, j);
                arma::Col<T1> in(in_ptr, in_size.n_rows, false, true);

                auto xcorr = correlate::xcorr(in, correlate::scale::unbiased);
                xcorr = xcorr(in_size.n_cols - 1, xcorr.n_elem -1);

                auto idx_peaks = peaks::local_peaks(xcorr, radius_);
                /* TODO: add threshold <20-02-20, cneyton> */

                arma::uword idx_max;
                bool        idx_set = false;
                for (arma::uword idx: idx_peaks) {
                    if (static_cast<T3>(idx) >= period_range_.min ||
                        static_cast<T3>(idx) <= period_range_.max ) {
                        idx_max = idx;
                        idx_set = true;
                        break;
                    }
                }

                T2 fhr;
                T3 corrcoef;
                if (!idx_set) {
                    fhr      = 0;
                    corrcoef = 0;
                } else {
                    auto [xmax, ymax] = interp::vertex(xcorr, idx_max);
                    fhr      = 1/xmax;
                    corrcoef = ymax / xcorr[0];
                }

                (*out0_chunk)(j, k, 1) = fhr;
                (*out1_chunk)(j, k, 1) = corrcoef;
            }
        }

        auto output0 = dynamic_cast<Link<T2>*>(outputs_.at(0));
        auto output1 = dynamic_cast<Link<T3>*>(outputs_.at(1));
        output0->push(out0_chunk);
        output1->push(out1_chunk);
        return 0;
    }

private:
    arma::uword  radius_;
    T3           threshold_;
    period_range period_range_;
};

} /* namespace filter */

#endif /* FHR_FILTER_H */
