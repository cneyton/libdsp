#ifndef FILTER_H
#define FILTER_H

#include <chrono>
#include <string>
#include <vector>
#include <deque>

#include "common/log.h"

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

    int add_input(LinkInterface& link)
    {
        inputs_.push_back(&link);
        return 0;
    }

    int add_output(LinkInterface& link)
    {
        outputs_.push_back(&link);
        return 0;
    }

    virtual int activate() = 0;

    bool is_ready() const {return ready_;}
    void set_ready()      {ready_ = true;}
    void reset_ready()    {ready_ = false;}

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
    std::chrono::duration<double> get_mean_exec_time() const
    {
        std::chrono::duration<double> tot =
            std::accumulate(stats_.durations.begin(), stats_.durations.end(),
                            std::chrono::duration<double>::zero());
        if (stats_.n_execs == 0) return std::chrono::duration<double>::zero();
        else return tot/stats_.n_execs;
    }

    std::string get_name() const {return name_;}

protected:
    std::string name_;

    Pipeline * pipeline_;
    std::vector<LinkInterface*> inputs_;
    std::vector<LinkInterface*> outputs_;

    bool ready_   = false;
    bool verbose_ = false;

private:
    struct stats
    {
        arma::uword n_execs;
        std::deque<std::chrono::duration<double>> durations;
    } stats_;
};

#endif /* FILTER_H */
