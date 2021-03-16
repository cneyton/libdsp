#pragma once

#include "sigpack.h"
#include "filter.h"
#include "link.h"

namespace dsp::filter {


/**
 * @brief Compute the doppler frequency.
 *
 * @tparam T1 Input data type
 * @tparam T2 Output data type
 */
template<typename T1 = arma::cx_double, typename T2 = double>
class FD: public Filter
{
public:
    FD(common::Logger logger, std::string_view name, arma::uword nfft, arma::vec window):
        Filter(logger, name), nfft_(nfft),
        fftw_(sp::FFTW(nfft, FFTW_MEASURE)), window_(window)
    {
        Pad in  {.name = "in" , .format = Format()};
        Pad out {.name = "out", .format = Format()};
        input_pads_.insert({in.name, in});
        output_pads_.insert({out.name, out});
    }

    FD(common::Logger logger, arma::uword nfft, arma::vec window):
        FD(logger, "fd", nfft, window)
    {
    }

    int activate() override
    {
        log_debug(logger_, "{} filter activated", name_);

        auto input    = dynamic_cast<Link<T1>*>(inputs_.at("in"));
        auto output   = dynamic_cast<Link<T2>*>(outputs_.at("out"));
        auto chunk_in = std::make_shared<Chunk<T1>>();

        if (!input->pop(chunk_in)) {
            if (input->eof())
                output->eof_reached();
            return 0;
        }

        auto fmt_in    = input->format();
        auto fmt_out   = output->format();
        auto chunk_out = std::make_shared<Chunk<T2>>(fmt_out.n_rows, fmt_out.n_cols,
                                                     fmt_out.n_slices);

        /* TODO: add zero padding and windowing  <26-03-20, cneyton> */
        arma::Col<T2> w   = arma::regspace<arma::Col<T2>>(0, nfft_-1) - static_cast<T2>(nfft_)/2;
        arma::uword shift = nfft_/2;
        T2 scale          = 1/static_cast<T2>(nfft_);
        for (arma::uword k = 0; k < fmt_in.n_slices; k++) {
            for (arma::uword j = 0; j < fmt_in.n_cols; j++) {
                in_ = chunk_in->slice(k).col(j);
                fftw_.fft_cx(in_, in_);
                in_ = arma::shift(in_, shift);
                arma::Col<T2> psd = arma::square(arma::abs(in_));
                T2 m0 = arma::sum(psd);
                T2 m1 = arma::dot(psd, w);
                (*chunk_out)(0, j, k) = scale * m1/m0;
            }
        }

        if (verbose_) {
            chunk_in->print();
            chunk_out->print();
        }

        output->push(chunk_out);

        return 1;
    }

    void reset() override
    {
    }

    Contract negotiate_format() override
    {
        auto fmt_in  = input_pads_["in"].format;
        auto fmt_out = output_pads_["out"].format;

        if (fmt_in.n_cols   != fmt_out.n_cols ||
            fmt_in.n_slices != fmt_out.n_slices ||
            fmt_out.n_rows  != 1) {
            return Contract::unsupported_format;
        }

        // Create fftw plan by first application of fft
        in_ = arma::Col<T1>(nfft_);
        fftw_.fft_cx(in_, in_);

        return Contract::supported_format;
    }

private:
    arma::uword   nfft_;
    sp::FFTW      fftw_;
    arma::vec     window_;
    arma::Col<T1> in_;
};

} /* namespace dsp::filter */
