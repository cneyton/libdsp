#include "filter.h"

Filter::Filter(common::Logger logger): logger_(logger)
{
}

Filter::Filter(common::Logger logger, std::string name): logger_(logger), name_(name)
{
}

