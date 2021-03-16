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
            for (auto it = filters_.cbegin(); it != filters_.cend(); ++it) {
                if (it->second->is_ready())
                    return true;
            }
            return false;
        });
}

Filter * Pipeline::add_filter(std::unique_ptr<Filter> filter)
{
    if (filters_.find(filter->name()) != filters_.end())
        throw dsp_error(Errc::duplicate_filter);
    Filter * handle = filter.get();
    filter->set_pipeline(this);
    filters_[filter->name()] = std::move(filter);
    return handle;
}

Filter * Pipeline::get_filter(const std::string& name)
{
    auto search = filters_.find(name);
    if (search != filters_.end())
        return search->second.get();
    else
        return nullptr;
}

Contract Pipeline::negotiate_format()
{
    for (auto it = filters_.cbegin(); it != filters_.cend(); ++it) {
        if (it->second->negotiate_format() != Contract::supported_format)
            return Contract::unsupported_format;
    }

    for (auto& l: links_) {
        if (l->negotiate_format() != Contract::supported_format)
            return Contract::unsupported_format;
    }

    return Contract::supported_format;
}

void Pipeline::print_stats()
{
    for (auto it = filters_.cbegin(); it != filters_.cend(); ++it) {
        auto& f = it->second;
        std::cout << "filter "       << f->name() << "\n"
            << "\tn_execs: "         << f->n_execs() << "\n"
            << "\ttotal exec time: " << f->total_exec_time().count() << " s\n"
            << "\tmean exec time: "  << f->mean_exec_time().count() << " s\n";
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
    for (auto it = filters_.cbegin(); it != filters_.cend(); ++it) {
        auto& f = it->second;
        if (f->is_ready()) {
            try {
#ifdef DSP_PROFILE
                auto start = std::chrono::high_resolution_clock::now();
#endif
                [[maybe_unused]] int ret = f->activate();
                f->reset_ready();
#ifdef DSP_PROFILE
                if (ret) {
                    auto stop = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double> diff = stop - start;
                    f->update_stats(diff);
                    update_stats(diff);
                }
#endif
            } catch (...) {
                log_error(logger_, "failed to activate filter {}", f->name());
                /* TODO: manage error (rethrow exception ?) <23-10-20, cneyton> */
            }
            return 1;
        }
    }
    return 0;
}

} /* namespace dsp */
