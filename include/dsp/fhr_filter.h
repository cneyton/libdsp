#pragma once

#include "link.h"
#include "filter.h"
#include "correlate.h"
#include "peaks.h"
#include "interp.h"

namespace dsp::filter {

template<typename T1 = double, typename T2 = double, typename T3 = T2>
class FHR: public Filter
{
public:
    FHR(common::Logger logger, std::string_view name,
        arma::uword radius, arma::uword period_max, T3 threshold):
        Filter(logger, name),
        radius_(radius), period_max_(period_max), threshold_(threshold)
    {
        Pad in {.name = "in", .format = Format()};
        Pad fhr {.name = "fhr", .format = Format()};
        Pad cor {.name = "cor", .format = Format()};
        input_pads_.insert({in.name, in});
        output_pads_.insert({fhr.name, fhr});
        output_pads_.insert({cor.name, cor});
    }

    FHR(common::Logger logger, arma::uword radius, arma::uword period_max, T3 threshold):
        FHR(logger, "fhr", radius, period_max, threshold)
    {
    }

    int activate() override
    {
        log_debug(logger_, "fhr filter {} activated", name_);

        auto input      = dynamic_cast<Link<T1>*>(inputs_.at("in"));
        auto output_fhr = dynamic_cast<Link<T2>*>(outputs_.at("fhr"));
        auto output_cor = dynamic_cast<Link<T3>*>(outputs_.at("cor"));
        auto chunk_in   = std::make_shared<Chunk<T1>>();

        if (!input->pop(chunk_in)) {
            if (input->eof()) {
                output_fhr->eof_reached();
                output_cor->eof_reached();
            }
            return 0;
        }

        auto fmt_in    = input->format();
        auto fmt_out   = output_fhr->format();

        auto chunk_fhr = std::make_shared<Chunk<T2>>(fmt_out.n_rows, fmt_out.n_cols,
                                                     fmt_out.n_slices);
        auto chunk_cor = std::make_shared<Chunk<T3>>(fmt_out.n_rows, fmt_out.n_cols,
                                                     fmt_out.n_slices);

        for (uint k = 0; k < fmt_in.n_slices; k++) {
            for (uint j = 0; j < fmt_in.n_cols; j++) {
                auto col_ptr  = chunk_in->slice_colptr(k, j);
                arma::Col<T1> x(col_ptr, fmt_in.n_rows, false, true);

                auto xcorr = correlate::xcorr(x, correlate::scale::unbiased);
                xcorr = xcorr.subvec(x.n_elem - 1, xcorr.n_elem -1);

                arma::Col<T1> xcorr_part = xcorr.subvec(radius_ - 1, xcorr.n_elem - 1);
                auto idx_peaks = peaks::local_peaks(xcorr_part, radius_);
                idx_peaks = idx_peaks + radius_ - 1;
                idx_peaks = idx_peaks(arma::find(idx_peaks <= period_max_));

                arma::uvec  even_idx;
                arma::uvec  odd_idx;
                arma::uword idx_max;
                T2          fhr;
                T3          corrcoef;
                if (idx_peaks.n_elem <= 2) {
                    fhr      = 0;
                    corrcoef = 0;
                } else {
                    even_idx = idx_peaks(arma::regspace<arma::uvec>(0, 2, idx_peaks.n_elem-1));
                    odd_idx  = idx_peaks(arma::regspace<arma::uvec>(1, 2, idx_peaks.n_elem-1));
                    if (arma::mean(xcorr(even_idx)) / arma::mean(xcorr(odd_idx)) < threshold_) {
                        idx_max = idx_peaks.at(1);
                    } else {
                        idx_max = idx_peaks.at(0);
                    }
                    try {
                        auto res = interp::vertex(xcorr, idx_max);
                        fhr      = 1/res[0];
                        corrcoef = res[1] / xcorr[0];
                    } catch (...) {
                        fhr      = 0;
                        corrcoef = 0;
                    }
                }
                (*chunk_fhr)(0, j, k) = fhr;
                (*chunk_cor)(0, j, k) = corrcoef;
            }
        }

        output_fhr->push(chunk_fhr);
        output_cor->push(chunk_cor);

        return 1;
    }

    void reset() override
    {
        // nothing to do
    }

    Contract negotiate_format() override
    {
        auto fmt_in  = input_pads_["in"].format;
        auto fmt_fhr = output_pads_["fhr"].format;
        auto fmt_cor = output_pads_["cor"].format;

        if (fmt_in.n_cols   != fmt_fhr.n_cols ||
            fmt_in.n_slices != fmt_fhr.n_slices ||
            fmt_fhr.n_rows  != 1 ||
            fmt_fhr         != fmt_cor) {
            return Contract::unsupported_format;
        }

        return Contract::supported_format;
    }

private:
    arma::uword  radius_;
    arma::uword  period_max_;
    T3           threshold_;
};

} /* namespace dsp::filter */
