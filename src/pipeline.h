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
    virtual ~Pipeline() {}

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
        for (auto& filter: filters_)
            filter->set_ready();

        return 0;
    }

    int add_filter(std::unique_ptr<Filter> filter)
    {
        filters_.push_back(std::move(filter));
        return 0;
    }

    template<typename T>
    int link(Filter * src, Filter * dst)
    {
        links_.push_back(std::make_unique<Link<T>>(logger_, src, dst));
        return 0;
    }

private:
    std::vector<std::unique_ptr<Filter>>         filters_;
    std::vector<std::unique_ptr<LinkInterface>>  links_;
};

#endif
