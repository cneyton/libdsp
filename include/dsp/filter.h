#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <deque>

#include <armadillo>

#include "common/log.h"

namespace dsp {

class LinkInterface;
class Pipeline;

class Filter: public common::Log
{
public:
    Filter(common::Logger logger, std::string_view name): Log(logger), name_(name) {}
    virtual ~Filter() = default;

    void add_input(LinkInterface * link)
    {
        inputs_.push_back(link);
    }

    void add_output(LinkInterface * link)
    {
        outputs_.push_back(link);
    }

    /**
     * This method is called when something must be done in a filter. The
     * definition of that something depends on the semantic of the filter.
     *
     * \return 1 if the filter was activated
     *         0 if the filter wasn't activated
     */
    virtual int activate() = 0;

    virtual void reset()   = 0;

    /**
     *
     */
    //virtual void set_format() = 0;

    bool is_ready() const noexcept {return ready_;}
    void set_ready()      noexcept {ready_ = true;}
    void reset_ready()    noexcept {ready_ = false;}

    std::string name() const {return name_;}

    Pipeline * pipeline() const {return pipeline_;}
    void       set_pipeline(Pipeline * p) {pipeline_ = p;}

    //size_t n_input_pads()  const {return input_pads_.size();}
    //size_t n_output_pads() const {return output_pads.size();}

    // debug methods -----------------------------------------------------------
    void set_verbose()    {verbose_ = true;}

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

    std::chrono::duration<double> mean_exec_time() const
    {
        if (stats_.n_execs == 0) return std::chrono::duration<double>::zero();
        else return total_exec_time()/stats_.n_execs;
    }
    // -------------------------------------------------------------------------

protected:
    std::string name_;

    Pipeline * pipeline_;
    std::vector<LinkInterface*> inputs_;
    std::vector<LinkInterface*> outputs_;
    //std::vector<Pad>            input_pads_;
    //std::vector<Pad>            output_pads;

    bool ready_   = false;
    bool verbose_ = false;

private:
    struct
    {
        arma::uword n_execs = 0;
        /* TODO: replace w/ vector <23-10-20, cneyton> */
        std::deque<std::chrono::duration<double>> durations;
    } stats_;
};

} /* namespace dsp */
