#include "filter.h"

Filter::Filter(common::Logger logger): Log(logger)
{
}

Filter::Filter(common::Logger logger, std::string name): Log(logger), name_(name)
{
}

int Filter::add_input(Link& link)
{
    inputs_.push_back(&link);
    return 0;
}

int Filter::add_output(Link& link)
{
    outputs_.push_back(&link);
    return 0;
}

