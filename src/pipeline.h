#ifndef PIPELINE_H
#define PIPELINE_H

#include <vector>
#include <memory>
#include "common/log.h"

class Filter;

class Pipeline
{
public:
    int run();
    int add_filter(Filter filter);
protected:
    common::Logger logger_;
private:
    std::vector<std::unique_ptr<Filter>> filters_;
};

#endif
