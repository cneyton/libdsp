#pragma once

#include "filter.h"
#include "link.h"

namespace dsp::filter {

template<typename T, arma::uword N>
class Tee: public Filter
{
public:
    Tee(common::Logger logger): Filter(logger, "tee")
    {
        Pad in  {.name = "in" , .format = Format()};
        input_pads_.insert({in.name, in});

        for (arma::uword i = 0; i < N; ++i) {
            Pad out {.name = std::to_string(i), .format = Format()};
            output_pads_.insert({out.name, out});
        }
    }

    int activate() override
    {
        log_debug(logger_, "filter {} activated", this->name_);
        auto input    = dynamic_cast<Link<T>*>(inputs_.at("in"));
        auto chunk_in = std::make_shared<Chunk<T>>();

        if (!input->pop(chunk_in)) {
            if (input->eof()) {
                for (arma::uword i = 0; i < N; ++i) {
                    auto output = dynamic_cast<Link<T>*>(outputs_.at(std::to_string(i)));
                    output->eof_reached();
                }
            }
            return 0;
        }

        for (arma::uword i = 0; i < N; ++i) {
            auto output = dynamic_cast<Link<T>*>(outputs_.at(std::to_string(i)));
            output->push(chunk_in);
        }
        return 1;
    }

    void reset() override
    {
        // nothing to do
    }

    Contract negotiate_format() override
    {
        for (arma::uword i = 0; i < N; ++i) {
            if (input_pads_["in"].format != output_pads_[std::to_string(i)].format)
                return Contract::unsupported_format;
        }

        return Contract::supported_format;
    }
};

} /* namespace dsp::filter */
