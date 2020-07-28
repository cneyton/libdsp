#pragma once

#include "filter.h"
#include "link.h"

namespace dsp::filter
{

template<typename T>
class sink: public Filter
{
public:
    sink(common::Logger logger): common::Log(logger), Filter(logger, "sink") {}

    virtual int activate()
    {
        log_debug(logger_, "{} filter activated", name_);

        auto input = dynamic_cast<Link<T>*>(inputs_.at(0));

        auto chunk = std::make_shared<Chunk<T>>();
        if (!input->pop(chunk))
            return 0;

        if (verbose_)
            chunk->print();
        return 1;
    }
};

} /* namespace dsp::filter */
