#include "source_filter.h"

namespace filter
{

Source::Source(common::Logger logger, common::data::Handler * data_handler):
    Filter(logger), common::data::Consumer(logger, data_handler)
{
}

Source::~Source()
{
}

int Source::activate()
{
    return 0;
}

int Source::pop_us(common::data::ByteBuffer& buf)
{
    int ret;
    ret = pop(common::data::type::us, buf);
    return 0;
}

}
