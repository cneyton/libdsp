#include "pipeline.h"
#include "filter.h"
#include <memory>

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
    links_.push_back(std::make_unique<Link>(logger_, src, dst));
    return 0;
}

int Pipeline::run()
{
    int ret;
    for (auto& filter: filters_) {
        if (filter->is_ready()) {
            ret = filter->activate();
            common_die_zero(logger_, ret, -1, "failed to activate filter {}", filter->get_name());
            filter->reset_ready();
            return 1;
        }
    }
    // rerun all the filters to reactivate the sources
    std::for_each(filters_.begin(), filters_.end(), [](Filter* f){f->set_ready();});
    return 0;
}