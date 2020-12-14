#pragma once

#include "filter.h"
#include "link.h"

namespace dsp::filter {

template<typename T, arma::uword N>
class Tee: public Filter
{
public:
    Tee(common::Logger logger): Filter(logger, "tee") {}

    int activate() override
    {
        log_debug(logger_, "filter {} activated", this->name_);
        auto input    = dynamic_cast<Link<T>*>(inputs_.at(0));
        auto chunk_in = std::make_shared<Chunk<T>>();

        if (!input->pop(chunk_in)) {
            if (input->eof()) {
                for (arma::uword i = 0; i < N; ++i) {
                    auto output = dynamic_cast<Link<T>*>(outputs_.at(i));
                    output->eof_reached();
                }
            }
            return 0;
        }

        for (arma::uword i = 0; i < N; ++i) {
            auto output = dynamic_cast<Link<T>*>(outputs_.at(i));
            output->push(chunk_in);
        }
        return 1;
    }

    void reset() override
    {
        // nothing to do
    }
};

} /* namespace dsp::filter */
