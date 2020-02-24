#ifndef PIPELINE_H
#define PIPELINE_H

#include <vector>
#include <memory>
#include <chrono>
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
        return 0;
    }

    int run_once()
    {
        int ret;
        for (auto& filter: filters_) {
            if (filter->is_ready()) {
#ifdef DSP_PROFILE
                auto start = std::chrono::high_resolution_clock::now();
#endif
                ret = filter->activate();
                common_die_zero(logger_, ret, -1,
                                "failed to activate filter {}", filter->get_name());
                filter->reset_ready();
#ifdef DSP_PROFILE
                auto stop = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> diff = stop - start;
                filter->update_stats(diff);
#endif
                }
                return 1;
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

    void print_stats()
    {
        for (auto& f: filters_) {
            std::cout << "filter " << f->get_name() << std::endl;
            std::cout << "   n_execs: " << f->get_n_execs() << std::endl;
            std::cout << "   mean exec time: "
                << f->get_mean_exec_time().count() << "s" << std::endl;
        }
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
