#pragma once

#include <condition_variable>
#include <mutex>
#include <vector>
#include <memory>
#include <chrono>
#include "common/log.h"

#include "filter.h"
#include "link.h"
#include "exception.h"

namespace dsp
{

class Pipeline: public common::Log
{
public:
    Pipeline(common::Logger logger): Log(logger)
    {
        reset_stats();
    }

    void reset()
    {
        for (auto& l: links_) {
            l->reset_eof();
        }
    }

    void run()
    {
        while (run_once()) { }
    }

    void stop()
    {
        {
            std::unique_lock<std::mutex> lk(mutex_);
            run_ = false;
        }
        cond_.notify_all();
    }

    void wakeup()
    {
        {
            std::unique_lock<std::mutex> lk(mutex_);
        }
        cond_.notify_all();
    }

    void add_filter(std::unique_ptr<Filter> filter)
    {
        filter->set_pipeline(this);
        filters_.push_back(std::move(filter));
    }

    void print_stats()
    {
        for (auto& f: filters_) {
            std::cout << "filter " << f->get_name() << "\n"
                      << "   n_execs: " << f->get_n_execs() << "\n"
                      << "   tot exec time: " << f->get_tot_exec_time().count() << " s\n"
                      << "   mean exec time: " << f->get_mean_exec_time().count() << " s\n";
        }
        std::cout << "------------------------------\n"
                  << "Pipeline\n"
                  << "   n_execs: " << get_n_execs() << "\n"
                  << "   tot exec time: " << get_tot_exec_time().count() << " s\n";
    }

    template<typename T>
    void link(Filter& src, Filter& dst, arma::SizeCube& format)
    {
        if (src.get_pipeline() != this || dst.get_pipeline() != this)
            throw pipeline_error("src or dst are not part of the pipeline");

        //if (src_pad_nb >= src->get_nb_input_pads() ||
            //dst_pad_nb >= dst->get_nb_output_pads())
            //common_die(logger_, -3, "invalid pad number");

        // compare format
        //auto src_pad = src->get_pad(src_pad_nb);
        //auto dst_pad = dst->get_pad(dst_pad_nb);

        // link
        auto link = std::make_unique<Link<T>>(logger_, &src, &dst, format);
        links_.push_back(std::move(link));
    }

private:
    std::vector<std::unique_ptr<Filter>>         filters_;
    std::vector<std::unique_ptr<LinkInterface>>  links_;

    bool run_ = false;
    bool eof_ = true;
    std::condition_variable cond_;
    std::mutex              mutex_;

    struct stats
    {
        arma::uword n_execs;
        std::deque<std::chrono::duration<double>> durations;
    } stats_;

    void update_stats(std::chrono::duration<double>& duration)
    {
        stats_.n_execs++;
        stats_.durations.push_back(duration);
    }

    void reset_stats()
    {
        stats_.n_execs = 0;
        stats_.durations.clear();
    }

    arma::uword get_n_execs() const {return stats_.n_execs;}
    std::chrono::duration<double> get_tot_exec_time() const
    {
        return std::accumulate(stats_.durations.begin(), stats_.durations.end(),
                               std::chrono::duration<double>::zero());
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
                if (ret) {
                    auto stop = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double> diff = stop - start;
                    filter->update_stats(diff);
                    update_stats(diff);
                }
#endif
                return 1;
            }
        }
        return 0;
    }

    void wait()
    {
        std::unique_lock<std::mutex> lk(mutex_);
        cond_.wait(lk);
    }
};

} /* namespace dsp */
