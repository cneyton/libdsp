#pragma once

#include <condition_variable>
#include <mutex>
#include <vector>
#include <map>
#include <memory>
#include <chrono>

#include "common/log.h"

#include "filter.h"
#include "link.h"
#include "dsp_error.h"

namespace dsp {

class Pipeline: public common::Log
{
public:
    Pipeline(common::Logger logger);

    /**
     * @brief Reset filters & links.
     *
     * For each filter call the reset method and reset eof on
     * each link.
     */
    void reset();

    /**
     * @brief Run until there is no more filter to activate
     */
    void run();

    /**
     * @brief Set eof on each link
     */
    void stop();

    /**
     * @brief Wait for a filter in the pipeline to be ready
     */
    void wait();

    /**
     * @brief Notify the pipeline to stop waiting
     */
    void wakeup();

    /**
     * @brief Add a filter to the pipeline.
     *
     * Transfers ownership of the pointer to the pipeline
     *
     * @return Handle to the filter
     */
    /* TODO: maybe this step could be skipped if we do it during link <13-01-21, cneyton> */
    Filter * add_filter(std::unique_ptr<Filter> filter);

    /**
     * @brief Get the filter with the given name from the pipeline.
     *
     * @return Handle to the filter if it's in the pipeline
     *         nullptr otherwise
     */
    Filter * get_filter(const std::string& name);

    Contract negotiate_format();

    void print_stats();

    template<typename T>
    void link(Filter * src, const std::string& src_pad_name,
              Filter * dst, const std::string& dst_pad_name)
    {
        if (src->pipeline() != this || dst->pipeline() != this)
            throw dsp_error(Errc::link_forbidden);

        // link
        auto link = std::make_unique<Link<T>>(src, src_pad_name, dst, dst_pad_name);
        links_.push_back(std::move(link));
    }

private:
    std::map<std::string, std::unique_ptr<Filter>> filters_;
    std::vector<std::unique_ptr<LinkInterface>>    links_;

    std::condition_variable cv_;
    std::mutex              mutex_;

    /**
     * @name Profiling helpers
     * @{ */
    struct
    {
        arma::uword n_execs;
        std::deque<std::chrono::duration<double>> durations;
    } stats_;

    void update_stats(std::chrono::duration<double>& duration);
    void reset_stats();

    arma::uword n_execs() const {return stats_.n_execs;}
    std::chrono::duration<double> total_exec_time() const;
    /**  @} */


    /**
     * Browse filters in the pipeline looking for filters ready to activate.
     * Returns after the activation of a filter or after browsing all filters.
     *
     * @return 1 if a filter was activated
     *         0 if no filter was activated
     */
    int run_once();
};

} /* namespace dsp */
