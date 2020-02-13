#ifndef PIPELINE_H
#define PIPELINE_H

#include <vector>
#include <memory>
#include "common/log.h"

#include "filter.h"
#include "link.h"

class Pipeline: public common::Log
{
public:
    Pipeline(common::Logger logger);
    virtual ~Pipeline();

    int run();
    int add_filter(Filter * filter);
    int link(Filter * src, Filter * dst);

private:
    std::vector<Filter*> filters_;
    std::vector<std::unique_ptr<Link>>    links_;
};

#endif
