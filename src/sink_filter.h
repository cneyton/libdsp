#ifndef SINK_FILTER_H
#define SINK_FILTER_H

#include "filter.h"

namespace filter
{

class Sink: public Filter
{
public:
    Sink(common::Logger logger);
    virtual ~Sink();

    virtual int activate();

private:
};

} /* namespace filter */

#endif /* SINK_FILTER_H */
