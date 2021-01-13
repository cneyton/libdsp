#include "dsp/pipeline.h"

namespace dsp {


Pipeline::Pipeline(common::Logger logger): Log(logger)
{
    reset_stats();
}

void Pipeline::reset()
{
    for (auto& l: links_) {
        l->reset_eof();
    }
    /* TODO: reset filters <30-10-20, cneyton> */
}

void Pipeline::run()
{
    while (run_once()) { }
}

void Pipeline::stop()
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

void Pipeline::wakeup()
{
    {
        std::unique_lock<std::mutex> lk(mutex_);
    }
    cv_.notify_all();
}

void Pipeline::wait()
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

Filter * Pipeline::add_filter(std::unique_ptr<Filter> filter)
{
    Filter * handle = filter.get();
    filter->set_pipeline(this);
    filters_.push_back(std::move(filter));
    return handle;
}

void Pipeline::print_stats()
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


void Pipeline::update_stats(std::chrono::duration<double>& duration)
{
    stats_.n_execs++;
    stats_.durations.push_back(duration);
}

void Pipeline::reset_stats()
{
    stats_.n_execs = 0;
    stats_.durations.clear();
}

std::chrono::duration<double> Pipeline::total_exec_time() const
{
    return std::accumulate(stats_.durations.begin(), stats_.durations.end(),
                           std::chrono::duration<double>::zero());
}

int Pipeline::run_once()
{
    for (auto& filter: filters_) {
        if (filter->is_ready()) {
#ifdef DSP_PROFILE
            auto start = std::chrono::high_resolution_clock::now();
#endif
            try {
                [[maybe_unused]] int ret = filter->activate();
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

} /* namespace dsp */
