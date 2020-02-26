#ifndef SINK_FILTER_H
#define SINK_FILTER_H

#include "filter.h"
#include "link.h"

namespace filter
{

template<typename T>
class sink: public Filter
{
public:
    sink(common::Logger logger): Log(logger), Filter(logger, "sink") {}
    virtual ~sink() {}

    virtual int activate()
    {
        log_debug(logger_, "sink {} activated", this->name_);

        auto input = dynamic_cast<Link<T>*>(inputs_.at(0));
        if (input->empty()) return 0;

        int ret;
        auto chunk = std::make_shared<Chunk<T>>();
        ret = input->front(chunk);
        common_die_zero(logger_, ret, -1, "failed to get front");
        ret = input->pop();
        common_die_zero(logger_, ret, -2, "failed to pop link");
        if (verbose_)
            chunk->print();
        return 1;
    }
};

} /* namespace filter */

#endif /* SINK_FILTER_H */
