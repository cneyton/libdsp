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
    Source(common::Logger logger, common::data::Handler * data_handler, common::data::type type);
    ~Source();

    virtual int activate();

    int set_format(uint16_t sample_size, uint16_t nb_samples, uint16_t nb_slots);
    int set_nb_frames(uint16_t nb_frames);

private:
    common::data::type type_;
    uint16_t sample_size_;
    uint16_t nb_samples_;
    uint16_t nb_slots_;
    uint16_t nb_frames_;
};

} /* namespace filter */

#endif /* SOURCE_FILTER_H */
