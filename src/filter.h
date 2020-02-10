#ifndef FILTER_H
#define FILTER_H

#include <string>
#include <vector>

#include "common/log.h"

#include "pipeline.h"

class Link;

class Filter: virtual public common::Log
{
public:
    Filter(common::Logger logger);
    Filter(common::Logger logger, std::string name);

    int add_input(Link& link);
    int add_output(Link& link);

    virtual int activate() = 0;

protected:
    std::string name_;

    Pipeline * pipeline_;
    std::vector<Link*> inputs_;
    std::vector<Link*> outputs_;

    uint16_t samplecount;
};

#endif /* FILTER_H */
