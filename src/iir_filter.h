#ifndef IIR_FILTER_H
#define IIR_FILTER_H

#include <vector>
#include <memory>
#include "sigpack.h"

#include "common/log.h"

#include "filter.h"
#include "link.h"

namespace filter
{

template<typename T1, typename T2>
class iir: public Filter
{
public:
    iir(common::Logger logger, uint16_t nb_filters, const arma::vec& b, const arma::vec& a):
        Log(logger), Filter(logger, "iir"), b_(b), a_(a),
        filters_(std::vector<sp::IIR_filt<T1, T2, T1>>(nb_filters))
    {
        /* TODO: do this at link <25-03-20, cneyton> */
        std::for_each(filters_.begin(), filters_.end(), [&](auto& f){f.clear();});
        std::for_each(filters_.begin(), filters_.end(), [&](auto& f){f.set_coeffs(b, a);});

        //input_pads_.push_back(input_pad_0);
        //output_pads.push_back(output_pad_0);
    }

    iir(common::Logger logger, uint16_t nb_filters):
        iir(logger, nb_filters,
            arma::Col<T2>(1, arma::fill::ones),
            arma::Col<T2>(1, arma::fill::ones))
    {
    }

    virtual ~iir() {}

    virtual int activate()
    {
        log_debug(logger_, "{} filter activated", this->name_);

        auto input    = dynamic_cast<Link<T1>*>(inputs_.at(0));
        auto output   = dynamic_cast<Link<T1>*>(outputs_.at(0));
        auto chunk_in = std::make_shared<Chunk<T1>>();

        int ret;
        ret = input->pop(chunk_in);
        common_die_zero(logger_, ret, -1, "failed to pop chunk");
        if (!ret) {
            if (input->eof())
                output->eof_reached();
            return 0;
        }

        auto size = arma::size(*chunk_in);
        auto chunk_out = std::make_shared<Chunk<T1>>(size);
        uint n = 0;
        for (uint k = 0; k < size.n_slices; k++) {
            for (uint j = 0; j < size.n_cols; j++) {
                auto in_ptr  = chunk_in->slice_colptr(k, j);
                auto out_ptr = chunk_out->slice_colptr(k, j);
                arma::Col<T1> in(in_ptr, size.n_rows, false, true);
                arma::Col<T1> out(out_ptr, size.n_rows, false, true);
                out = filters_[n].filter(in);
                n++;
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
        auto output  = dynamic_cast<Link<T1>*>(outputs_.at(0));
        auto fmt_in  = input->get_format();
        auto fmt_out = output->get_format();

        if (fmt_in != fmt_out)
            return 0;

        // Allocate and init filters
        filters_ = std::vector<sp::IIR_filt<T1, T2, T1>>(fmt_in.n_cols * fmt_in.n_slices);
        std::for_each(filters_.begin(), filters_.end(), [&](auto& f){f.clear();});
        std::for_each(filters_.begin(), filters_.end(), [&](auto& f){f.set_coeffs(b_, a_);});

        return 1;
    }

    virtual int clear()
    {
        std::for_each(filters_.begin(), filters_.end(), [&](auto& f){f.clear();});
        return 1;
    }

private:
    arma::Col<T2> b_;
    arma::Col<T2> a_;
    std::vector<sp::IIR_filt<T1, T2, T1>> filters_;

    //static const Pad<T1> input_pad {
        //.name = "input",
    //};

    //static const Pad<T1> output_pad {
        //.name = "output"
    //};
};

} /* namespace filter */

#endif /* IIR_FILTER_H */
