#include "pipeline.h"
#include "filter.h"

Pipeline::Pipeline(common::Logger logger): Log(logger)
{
}

Pipeline::~Pipeline()
{
    for (auto& filter: filters_) {
        delete filter;
    }
}

int Pipeline::add_filter(Filter * filter)
{
    filters_.push_back(filter);
    return 0;
}

int Pipeline::link(Filter * src, Filter * dst)
{
    links_.emplace_back(logger_, src, dst);
    return 0;
}

int Pipeline::run()
{
    int ret;
    for (auto& filter: filters_) {
        if (filter->is_ready()) {
            ret = filter->activate();
            common_die_zero(logger_, ret, -1, "failed to activate filter {}", filter->get_name());
        }
    }
    return 0;
}
