#ifndef FHR_FILTER_H
#define FHR_FILTER_H

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

    struct period_range
    {
        T3 min;
        T3 max;
    };

    fhr(common::Logger logger, arma::uword fdperseg, arma::uword fdskip, period_range range):
        Log(logger), Filter(logger, "fhr"), fdperseg_(fdperseg), fdskip_(fdskip),
        period_range_(range)
    {
    }
    virtual ~fhr() {}


    virtual int activate()
    {
        log_debug(logger_, "fhr filter {} activated", this->name_);

        auto input = dynamic_cast<Link<T1>*>(inputs_.at(0));
        if (input->size() < fdperseg_) return 0;

        int ret;
        auto in_chunk = input->head_chunk(fdperseg_);
        ret = input->pop_head(fdskip_);
        common_die_zero(logger_, ret, -1, "failed to pop head");

        arma::SizeCube in_size = arma::size(in_chunk);
        arma::SizeCube out_size(input->get_format());
        auto out0_chunk = std::make_shared<Chunk<T2>>(out_size);
        auto out1_chunk = std::make_shared<Chunk<T3>>(out_size);

        for (uint k = 0; k < in_size.n_slices; k++) {
            for (uint j = 0; j < in_size.n_cols; j++) {
                auto in_ptr  = in_chunk.slice_colptr(k, j);
                arma::Col<T1> in(in_ptr, in_size.n_rows, false, true);

                auto xcorr = correlate::xcorr(in, correlate::scale::unbiased);
                xcorr = xcorr.subvec(in_size.n_cols - 1, xcorr.n_elem -1);

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

                (*out0_chunk)(0, j, k) = fhr;
                (*out1_chunk)(0, j, k) = corrcoef;
            }
        }

        auto output0 = dynamic_cast<Link<T2>*>(outputs_.at(0));
        auto output1 = dynamic_cast<Link<T3>*>(outputs_.at(1));
        output0->push(out0_chunk);
        output1->push(out1_chunk);
        return 1;
    }

private:
    arma::uword  radius_;
    T3           threshold_;
    arma::uword  fdperseg_;
    arma::uword  fdskip_;
    period_range period_range_;
};

} /* namespace filter */

#endif /* FHR_FILTER_H */
