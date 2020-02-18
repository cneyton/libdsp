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
    Pipeline(common::Logger logger): Log(logger) {}

    virtual ~Pipeline()
    {
        for (auto& filter: filters_) {
            delete filter;
        }
    }

    int run()
    {
        int ret;
        for (auto& filter: filters_) {
            if (filter->is_ready()) {
                ret = filter->activate();
                common_die_zero(logger_, ret, -1,
                                "failed to activate filter {}", filter->get_name());
                filter->reset_ready();
                return 1;
            }
        }
        // rerun all the filters to reactivate the sources
        std::for_each(filters_.begin(), filters_.end(), [](Filter* f){f->set_ready();});
        return 0;
    }

    int add_filter(Filter * filter)
    {
        filters_.push_back(filter);
        return 0;
    }

    template<typename T>
    int link(Filter * src, Filter * dst)
    {
        links_.push_back(std::make_unique<Link<T>>(logger_, src, dst));
        return 0;
    }

private:
    std::vector<Filter*> filters_;
    std::vector<std::unique_ptr<LinkInterface>>    links_;
};

#endif
