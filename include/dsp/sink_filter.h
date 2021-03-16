#pragma once

#include "filter.h"
#include "link.h"

namespace dsp::filter {

template<typename T>
class Sink: public Filter
{
public:
    Sink(common::Logger logger, std::string_view name = "sink"):
        Filter(logger, name)
    {
        Pad p {.name="in", .format=Format()};
        input_pads_.insert({p.name, p});
    }

    int activate() override
    {
        log_debug(logger_, "{} filter activated", name_);

        auto input = dynamic_cast<Link<T>*>(inputs_.at("in"));

        auto chunk = std::make_shared<Chunk<T>>();
        if (!input->pop(chunk))
            return 0;

        if (verbose_)
            chunk->print();
        return 1;
    }

    void reset() override
    {
        // nothing to do
    }

    Contract negotiate_format() override
    {
        return Contract::supported_format;
    }
};

} /* namespace dsp::filter */
