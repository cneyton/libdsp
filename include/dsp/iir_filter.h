#pragma once

#include <vector>
#include <memory>
#include <filesystem>

#include "common/log.h"

#include "cnpy.h"
#include "sigpack.h"
#include "filter.h"
#include "link.h"

namespace dsp {
namespace filter {

template<typename T1, typename T2>
class IIR: public Filter
{
public:
    IIR(common::Logger logger, const arma::vec& b, const arma::vec& a):
        Filter(logger, "iir"),
        b_(b), a_(a)
    {
        Pad in  {.name = "in" , .format = Format()};
        Pad out {.name = "out", .format = Format()};
        input_pads_.insert({in.name, in});
        output_pads_.insert({out.name, out});
    }

    IIR(common::Logger logger):
        IIR(logger,
            arma::Col<T2>(1, arma::fill::ones),
            arma::Col<T2>(1, arma::fill::ones))
    {
    }

    int activate() override
    {
        log_debug(logger_, "{} filter activated", name_);

        auto input    = dynamic_cast<Link<T1>*>(inputs_.at("in"));
        auto output   = dynamic_cast<Link<T1>*>(outputs_.at("out"));
        auto chunk_in = std::make_shared<Chunk<T1>>();

        if (!input->pop(chunk_in)) {
            if (input->eof())
                output->eof_reached();
            return 0;
        }

        auto size = output->format();
        auto chunk_out = std::make_shared<Chunk<T1>>(size.n_rows, size.n_cols, size.n_slices);
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

    void reset() override
    {
        /* TODO: todo <30-10-20, cneyton> */
    }

    Contract negotiate_format() override
    {
        if (input_pads_["in"].format != output_pads_["out"].format)
            return Contract::unsupported_format;

        // Allocate and init filters
        auto fmt = input_pads_["in"].format;
        filters_ = std::vector<sp::IIR_filt<T1, T2, T1>>(fmt.n_cols * fmt.n_slices);
        std::for_each(filters_.begin(), filters_.end(), [&](auto& f){f.clear();});
        std::for_each(filters_.begin(), filters_.end(), [&](auto& f){f.set_coeffs(b_, a_);});

        return Contract::supported_format;
    }

    void clear()
    {
        std::for_each(filters_.begin(), filters_.end(), [&](auto& f){f.clear();});
    }

    void load_parameters(std::filesystem::path filename)
    {
        auto a = cnpy::npz_load(filename, "a");
        auto b = cnpy::npz_load(filename, "b");

        if (a.word_size != sizeof(T2) || b.word_size != sizeof(T2))
            throw dsp_error(Errc::invalid_parameters);

        a_ = arma::Col<T2>(a.data<T2>(), a.shape.at(0));
        b_ = arma::Col<T2>(b.data<T2>(), b.shape.at(0));
        std::for_each(filters_.begin(), filters_.end(), [&](auto& f){f.clear();});
        std::for_each(filters_.begin(), filters_.end(), [&](auto& f){f.set_coeffs(b_, a_);});
    }

private:
    arma::Col<T2> b_;
    arma::Col<T2> a_;
    std::vector<sp::IIR_filt<T1, T2, T1>> filters_;
};

} /* namespace filter */
} /* namespace dsp */
