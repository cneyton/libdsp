#ifndef FILTER_H
#define FILTER_H

#include <string>

#include "common/log.h"

class Filter
{
public:
    Filter(common::Logger logger);
    Filter(common::Logger logger, std::string name);

    virtual int activate() = 0;
protected:
    common::Logger logger_;
    std::string name_;
    uint32_t nb_inputs;
    uint16_t nb_outputs;
    uint16_t samplecount;
private:
};

#endif /* FILTER_H */
