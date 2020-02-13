#include "sink_filter.h"
#include "chunk.h"
#include "link.h"
#include <memory>

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
    auto chunk = std::make_shared<Chunk>(logger_);
    if (inputs_.at(0)->pop(chunk)) {
        if (verbose_) chunk->print();
    } else {
        ready_ = false;
    }
    return 0;
}

} /* namespace  filter */

