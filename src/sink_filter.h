#ifndef SINK_FILTER_H
#define SINK_FILTER_H

#include "filter.h"
#include "chunk.h"
#include "link.h"

namespace filter
{

template<typename T>
class sink: public Filter
{
public:
    sink(common::Logger logger): Log(logger), Filter(logger) {}
    virtual ~sink() {}

    virtual int activate()
    {
        log_debug(logger_, "sink {} activated", this->name_);

        auto chunk = std::make_shared<Chunk<T>>(logger_);
        auto input = dynamic_cast<Link<T>*>(inputs_.at(0));
        if (input->pop(chunk)) {
            if (verbose_) chunk->print();
        } else {
            ready_ = false;
        }
        return 0;
    }
};

} /* namespace filter */

#endif /* SINK_FILTER_H */
