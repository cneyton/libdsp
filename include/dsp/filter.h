#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <deque>

#include <armadillo>

#include "common/log.h"

namespace dsp
{

class LinkInterface;
class Pipeline;

class Filter: virtual public common::Log
{
public:
    Filter(common::Logger logger): Log(logger)
    {
        reset_stats();
    }

    Filter(common::Logger logger, std::string name): Filter(logger)
    {
        name_ = name;
    }

    virtual ~Filter() {}

    void add_input(LinkInterface& link)
    {
        inputs_.push_back(&link);
    }

    void add_output(LinkInterface& link)
    {
        outputs_.push_back(&link);
    }

    virtual int activate() = 0;
    //virtual void reset()   = 0;

    bool is_ready() const {return ready_;}
    void set_ready()      {ready_ = true;}
    void reset_ready()    {ready_ = false;}

    bool is_source() const {return source_;}

    std::string get_name() const {return name_;}

    Pipeline * get_pipeline() const {return pipeline_;}
    void       set_pipeline(Pipeline * p) {pipeline_ = p;}

    //uint get_nb_input_pads()  const {return input_pads_.size();}
    //uint get_nb_output_pads() const {return output_pads.size();}

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

    arma::uword get_n_execs() const {return stats_.n_execs;}

    std::chrono::duration<double> get_tot_exec_time() const
    {
        return std::accumulate(stats_.durations.begin(), stats_.durations.end(),
                            std::chrono::duration<double>::zero());
    }

    std::chrono::duration<double> get_mean_exec_time() const
    {
        if (stats_.n_execs == 0) return std::chrono::duration<double>::zero();
        else return get_tot_exec_time()/stats_.n_execs;
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
    bool source_  = false;

private:
    struct stats
    {
        arma::uword n_execs;
        std::deque<std::chrono::duration<double>> durations;
    } stats_;
};

} /* namespace dsp */
