#include "sink_filter.h"
#include "chunk.h"
#include "link.h"

namespace filter
{

Sink::Sink(common::Logger logger): Log(logger), Filter(logger)
{
}

Sink::~Sink()
{
}

int Sink::activate()
{
    Chunk chunk(logger_);
    if (inputs_.at(0)->pop(chunk))
        chunk.print();
    else
        ready_ = false;
    return 0;
}

} /* namespace  filter */

