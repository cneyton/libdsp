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
        arma::Col<T1> vec(nfft);
        fftw_.fft_cx(vec, vec);
    }

    virtual ~fd() {}

    virtual int activate()
    {
        log_debug(logger_, "fd filter {} activated", this->name_);

        auto input = dynamic_cast<Link<T1>*>(inputs_.at(0));
        if (input->empty()) return 0;

        int ret;
        auto in_chunk = std::make_shared<Chunk<T1>>();
        ret = input->front(in_chunk);
        common_die_zero(logger_, ret, -1, "failed to get front");
        ret = input->pop();
        common_die_zero(logger_, ret, -2, "failed to pop link");

        auto size = arma::size(*in_chunk);
        auto out_chunk = std::make_shared<Chunk<T2>>(arma::size(1, size.n_cols, size.n_slices));

        for (uint k = 0; k < size.n_slices; k++) {
            for (uint j = 0; j < size.n_cols; j++) {
                arma::Col<T1> in = in_chunk->slice(k).col(j);
                fftw_.fft_cx(in, in);
                arma::Col<T2> psd = arma::square(arma::abs(in));
                arma::Col<T2> w   = arma::regspace<arma::Col<T2>>(0, nfft_-1)
                    - static_cast<T2>(nfft_)/2;
                T2 m0 = arma::sum(psd);
                T2 m1 = arma::dot(psd, w);
                (*out_chunk)(0, j, k) = 1/static_cast<T2>(nfft_) * m1/m0;
            }
        }

        if (verbose_) {
            in_chunk->print();
            out_chunk->print();
        }

        auto output = dynamic_cast<Link<T2>*>(outputs_.at(0));
        output->push(out_chunk);

        return 1;
    }

private:
    arma::uword nfft_;
    sp::FFTW    fftw_;
    arma::vec   window_;
};

} /* namespace filter */

#endif /* FD_FILTER_H */
