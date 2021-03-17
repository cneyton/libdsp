#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <map>

#include <armadillo>

#include "common/log.h"

#include "format.h"

namespace dsp {

class LinkInterface;
class Pipeline;

struct Pad
{
    std::string    name;
    Format         format;
};

class Filter: public common::Log
{
public:
    Filter(common::Logger logger, std::string_view name);
    virtual ~Filter() = default;

    /**
     * @brief Add an input link to the filter
     *
     * @param link Pointer to the link to add
     * @param pad_name Pad name where the link will be added
     */
    void add_input(LinkInterface * link, const std::string& pad_name);

    /**
     * @brief Add an output link to the filter
     *
     * @param link Pointer to the link to add
     * @param pad_name Pad name where the link will be added
     */
    void add_output(LinkInterface * link, const std::string& pad_name);

    /**
     * @brief This method is called when something must be done in a filter.
     * The definition of that something depends on the semantic of the filter.
     *
     * @return 1 if the filter was activated
     *         0 if the filter wasn't activated
     */
    virtual int activate() = 0;

    /**
     * @brief Reset the filter to its initial state.
     */
    virtual void reset()   = 0;

    /**
     * @brief Checks if input & output formats are compatible.
     *
     * @return Contract::supported_format if input & output are compatible.
     *         Contract::unsupported_format is returned otherwise.
     */
    virtual Contract negotiate_format() = 0;

    void set_input_format(const Format& f, const std::string& pad_name);
    void set_output_format(const Format& f, const std::string& pad_name);
    const Format& get_input_format(const std::string& pad_name);
    const Format& get_output_format(const std::string& pad_name);

    bool is_ready() const noexcept {return ready_;}
    void set_ready()      noexcept {ready_ = true;}
    void reset_ready()    noexcept {ready_ = false;}

    std::string name() const {return name_;}

    Pipeline * pipeline() const {return pipeline_;}
    void       set_pipeline(Pipeline * p) {pipeline_ = p;}

    // debug methods -----------------------------------------------------------
    void set_verbose()    {verbose_ = true;}

    void update_stats(std::chrono::duration<double>& duration);
    void reset_stats();
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
