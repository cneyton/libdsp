#ifndef PIPELINE_H
#define PIPELINE_H

#include <condition_variable>
#include <mutex>
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
        int ret;
        run_ = true;
        while (run_) {
            ret = run_once();
            common_die_zero(logger_, ret, -1, "pipeline run error");
            if (ret == 0) {
                ret = wait();
                common_die_zero(logger_, ret, -2, "pipeline wait error");
            }
        }
        return 0;
    }


    int stop()
    {
        {
            std::unique_lock<std::mutex> lk(mutex_);
            run_ = false;
        }
        cond_.notify_all();
        return 0;
    }

    int resume()
    {
        {
            std::unique_lock<std::mutex> lk(mutex_);
            for (auto& filter: filters_)
                if (filter->is_source())
                    filter->set_ready();
        }
        cond_.notify_all();
        return 0;
    }


    int add_filter(std::unique_ptr<Filter> filter)
    {
        filter->set_pipeline(this);
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
    int link(Filter * src, Filter * dst, arma::SizeCube& format)
    {
        common_die_null(logger_, src, -1, "src nullptr");
        common_die_null(logger_, dst, -2, "dst nullptr");

        if (src->get_pipeline() != this || dst->get_pipeline() != this)
            common_die(logger_, -3, "src or dst are not part of the pipeline");

        //if (src_pad_nb >= src->get_nb_input_pads() ||
            //dst_pad_nb >= dst->get_nb_output_pads())
            //common_die(logger_, -3, "invalid pad number");

        // compare format
        //auto src_pad = src->get_pad(src_pad_nb);
        //auto dst_pad = dst->get_pad(dst_pad_nb);

        // link
        auto link = std::make_unique<Link<T>>(logger_, src, dst, format);
        links_.push_back(std::move(link));
        return 0;
    }

private:
    std::vector<std::unique_ptr<Filter>>         filters_;
    std::vector<std::unique_ptr<LinkInterface>>  links_;

    bool run_ = false;
    std::condition_variable cond_;
    std::mutex              mutex_;

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
                if (ret) {
                    auto stop = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double> diff = stop - start;
                    filter->update_stats(diff);
                }
#endif
                return 1;
            }
        }
        return 0;
    }

    int wait()
    {
        std::unique_lock<std::mutex> lk(mutex_);
        cond_.wait(lk);
        return 0;
    }
};

#endif
