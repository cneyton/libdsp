#ifndef FD_FILTER_H
#define FD_FILTER_H

#include "sigpack.h"
#include "filter.h"
#include "link.h"

namespace filter
{

template<typename T1 = arma::cx_double, typename T2 = double>
class fd: public Filter
{
public:
    fd(common::Logger logger, arma::uword nfft, arma::vec window):
        Log(logger), Filter(logger, "fd"), nfft_(nfft),
        fftw_(sp::FFTW(nfft, FFTW_MEASURE)), window_(window)
    {
        /* TODO: do this at link  <14-02-20, cneyton> */
        // Create fftw plan by first application of fft
        in_ = arma::Col<T1>(nfft);
        fftw_.fft_cx(in_, in_);
    }

    virtual ~fd() {}

    virtual int activate()
    {
        log_debug(logger_, "{} filter activated", this->name_);

        auto input    = dynamic_cast<Link<T1>*>(inputs_.at(0));
        auto output   = dynamic_cast<Link<T2>*>(outputs_.at(0));
        auto chunk_in = std::make_shared<Chunk<T1>>();

        int ret;
        ret = input->pop(chunk_in);
        common_die_zero(logger_, ret, -1, "failed to pop chunk");
        if (!ret) {
            if (input->eof())
                output->eof_reached();
            return 0;
        }

        auto fmt_in    = input->get_format();
        auto fmt_out   = output->get_format();
        auto chunk_out = std::make_shared<Chunk<T2>>(fmt_out);

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

    virtual int negotiate_fmt()
    {
        auto input   = dynamic_cast<Link<T1>*>(inputs_.at(0));
        auto output  = dynamic_cast<Link<T2>*>(outputs_.at(0));
        auto fmt_in  = input->get_format();
        auto fmt_out = output->get_format();

        if (fmt_in.n_cols != fmt_out.n_cols || fmt_in.n_slices != fmt_out.n_slices)
            return 0;

        if (fmt_out.n_rows != 1)
            return 0;

        // Create fftw plan by first application of fft
        in_ = arma::Col<T1>(nfft_);
        fftw_.fft_cx(in_, in_);

        return 1;
    }

private:
    arma::uword nfft_;
    sp::FFTW    fftw_;
    arma::vec   window_;
    arma::Col<T1>   in_;
};

} /* namespace filter */

#endif /* FD_FILTER_H */
