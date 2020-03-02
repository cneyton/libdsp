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
        Log(logger), Filter(logger, "iir"),
        filters_(std::vector<sp::IIR_filt<T1, T2, T1>>(nb_filters))
    {
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
        log_debug(logger_, "iir filter {} activated", this->name_);

        auto input = dynamic_cast<Link<T1>*>(inputs_.at(0));
        if (input->empty()) return 0;

        int ret;
        auto in_chunk = std::make_shared<Chunk<T1>>();
        ret = input->front(in_chunk);
        common_die_zero(logger_, ret, -1, "failed to get front");
        ret = input->pop();
        common_die_zero(logger_, ret, -2, "failed to pop link");

        auto size = arma::size(*in_chunk);
        auto out_chunk = std::make_shared<Chunk<T1>>(size, arma::fill::zeros);
        uint n = 0;
        for (uint k = 0; k < size.n_slices; k++) {
            for (uint j = 0; j < size.n_cols; j++) {
                auto in_ptr  = in_chunk->slice_colptr(k, j);
                auto out_ptr = out_chunk->slice_colptr(k, j);
                arma::Col<T1> in(in_ptr, size.n_rows, false, true);
                arma::Col<T1> out(out_ptr, size.n_rows, false, true);
                out = filters_[n].filter(in);
                n++;
            }
        }

        if (verbose_) {
            in_chunk->print();
            out_chunk->print();
        }

        auto output = dynamic_cast<Link<T1>*>(outputs_.at(0));
        output->push(out_chunk);
        return 1;
    }

private:
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
