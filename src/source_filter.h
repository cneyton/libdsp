#ifndef SOURCE_FILTER_H
#define SOURCE_FILTER_H

#include "common/log.h"
#include "common/data.h"
#include "filter.h"

namespace filter
{

class Source: public Filter, public common::data::Consumer
{
public:
    Source(common::Logger logger, common::data::Handler * data_handler);
    ~Source();

    virtual int activate();

    int pop_us(common::data::ByteBuffer& buf);

private:
};

} /* namespace filter */

#endif /* SOURCE_FILTER_H */
