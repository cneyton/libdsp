#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <deque>
#include <map>

#include <armadillo>

#include "common/log.h"

#include "dsp_error.h"
#include "format.h"
#include "pad.h"

namespace dsp {

class LinkInterface;
class Pipeline;

class Filter: public common::Log
{
public:
    Filter(common::Logger logger, std::string_view name): Log(logger), name_(name) {}
    virtual ~Filter() = default;

    /**
     * @brief Add an input link to the filter
     *
     * @param link Pointer to the link to add
     * @param pad_name Pad name where the link will be added
     */
    void add_input(LinkInterface * link, const std::string& pad_name)
    {
        if (input_pads_.find(pad_name) == input_pads_.end())
            throw dsp_error(Errc::pad_unknown);
        if (inputs_.find(pad_name) != inputs_.end())
            throw dsp_error(Errc::pad_occupied);
        inputs_[pad_name]  = link;
    }

    /**
     * @brief Add an output link to the filter
     *
     * @param link Pointer to the link to add
     * @param pad_name Pad name where the link will be added
     */
    void add_output(LinkInterface * link, const std::string& pad_name)
    {
        if (output_pads_.find(pad_name) == output_pads_.end())
            throw dsp_error(Errc::pad_unknown);
        if (outputs_.find(pad_name) != outputs_.end())
            throw dsp_error(Errc::pad_occupied);
        outputs_[pad_name]  = link;
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
     * @brief This method checks if input & output formats are compatible.
     *
     * @return Contract::supported_format if input & output are compatible.
     *         Contract::unsupported_format is returned otherwise.
     */
    virtual Contract negotiate_format() = 0;

    void set_input_format(const Format& f, const std::string& pad_name)
    {
        if (input_pads_.find(pad_name) == input_pads_.end())
            throw dsp_error(Errc::pad_unknown);
        input_pads_[pad_name].format = f;
    }

    void set_output_format(const Format& f, const std::string& pad_name)
    {
        if (output_pads_.find(pad_name) == output_pads_.end())
            throw dsp_error(Errc::pad_unknown);
        output_pads_[pad_name].format = f;
    }

    const Format& get_input_format(const std::string& pad_name)
    {
        if (input_pads_.find(pad_name) == input_pads_.end())
            throw dsp_error(Errc::pad_unknown);
        return input_pads_[pad_name].format;
    }

    const Format& get_output_format(const std::string& pad_name)
    {
        if (output_pads_.find(pad_name) == output_pads_.end())
            throw dsp_error(Errc::pad_unknown);
        return output_pads_[pad_name].format;
    }

    bool is_ready() const noexcept {return ready_;}
    void set_ready()      noexcept {ready_ = true;}
    void reset_ready()    noexcept {ready_ = false;}

    std::string name() const {return name_;}

    Pipeline * pipeline() const {return pipeline_;}
    void       set_pipeline(Pipeline * p) {pipeline_ = p;}

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
        return std::accumulate(stats_.durations.cbegin(), stats_.durations.cend(),
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
    // maps pad names to filter links
    std::map<std::string, LinkInterface*>  inputs_;
    std::map<std::string, LinkInterface*>  outputs_;
    // maps pad names to pads
    std::map<std::string, Pad>  input_pads_;
    std::map<std::string, Pad>  output_pads_;

    bool ready_   = false;
    bool verbose_ = false;

private:
    struct
    {
        arma::uword n_execs = 0;
        std::vector<std::chrono::duration<double>> durations;
    } stats_;
};

} /* namespace dsp */
