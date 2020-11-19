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

namespace dsp {

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
        /* TODO: reset filters <30-10-20, cneyton> */
    }

    void run()
    {
        while (run_once()) { }
    }

    void stop()
    {
        {
            std::unique_lock<std::mutex> lk(mutex_);
            // set eof on all links
            for (auto& l: links_) {
                l->eof();
            }
        }
        cv_.notify_all();
    }

    void wakeup()
    {
        {
            std::unique_lock<std::mutex> lk(mutex_);
        }
        cv_.notify_all();
    }

    void wait()
    {
        std::unique_lock<std::mutex> lk(mutex_);
        cv_.wait(lk, [this]()
            {
                for (auto& f: filters_) {
                    if (f->is_ready())
                        return true;
                }
                return false;
            });
    }

    /**
     * Add a filter to the pipeline.
     * Transfers ownership of the pointer to the pipeline
     * \return Handle to the filter
     */
    Filter * add_filter(std::unique_ptr<Filter> filter)
    {
        Filter * handle = filter.get();
        filter->set_pipeline(this);
        filters_.push_back(std::move(filter));
        return handle;
    }

    void print_stats()
    {
        for (auto& f: filters_) {
            std::cout << "filter " << f->name() << "\n"
                      << "\tn_execs: " << f->n_execs() << "\n"
                      << "\ttotal exec time: " << f->total_exec_time().count() << " s\n"
                      << "\tmean exec time: " << f->mean_exec_time().count() << " s\n";
        }
        std::cout << "------------------------------\n"
                  << "Pipeline\n"
                  << "\tn_execs: " << n_execs() << "\n"
                  << "\ttot exec time: " << total_exec_time().count() << " s\n";
    }

    template<typename T>
    void link(Filter * src, Filter * dst, arma::SizeCube& format)
    {
        if (src->pipeline() != this || dst->pipeline() != this)
            throw pipeline_error("src or dst are not part of the pipeline");

        //if (src_pad_nb >= src->get_nb_input_pads() ||
            //dst_pad_nb >= dst->get_nb_output_pads())
            //common_die(logger_, -3, "invalid pad number");

        // compare format
        //auto src_pad = src->get_pad(src_pad_nb);
        //auto dst_pad = dst->get_pad(dst_pad_nb);

        // link
        auto link = std::make_unique<Link<T>>(src, dst, format);
        links_.push_back(std::move(link));
    }

private:
    std::vector<std::unique_ptr<Filter>>         filters_;
    std::vector<std::unique_ptr<LinkInterface>>  links_;

    std::condition_variable cv_;
    std::mutex              mutex_;

    struct
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

    arma::uword n_execs() const {return stats_.n_execs;}
    std::chrono::duration<double> total_exec_time() const
    {
        return std::accumulate(stats_.durations.begin(), stats_.durations.end(),
                               std::chrono::duration<double>::zero());
    }

    /**
     *
     * \return 1 if a filter was activated
     *         0 if no filter was activated
     */
    int run_once()
    {
        for (auto& filter: filters_) {
            if (filter->is_ready()) {
#ifdef DSP_PROFILE
                auto start = std::chrono::high_resolution_clock::now();
#endif
                try {
                    int ret = filter->activate();
                } catch (...) {
                    log_error(logger_, "failed to activate filter {}", filter->name());
                    /* TODO: manage error (rethrow exception ?) <23-10-20, cneyton> */
                }
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
};

} /* namespace dsp */
