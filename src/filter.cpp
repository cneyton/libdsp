#include "dsp/filter.h"
#include "dsp/dsp_error.h"

namespace dsp {

Filter::Filter(common::Logger logger, std::string_view name):
    Log(logger), name_(name)
{
}

void Filter::add_input(LinkInterface * link, const std::string& pad_name)
{
    if (input_pads_.find(pad_name) == input_pads_.end())
        throw dsp_error(Errc::pad_unknown);
    if (inputs_.find(pad_name) != inputs_.end())
        throw dsp_error(Errc::pad_occupied);
    inputs_[pad_name]  = link;
}

void Filter::add_output(LinkInterface * link, const std::string& pad_name)
{
    if (output_pads_.find(pad_name) == output_pads_.end())
        throw dsp_error(Errc::pad_unknown);
    if (outputs_.find(pad_name) != outputs_.end())
        throw dsp_error(Errc::pad_occupied);
    outputs_[pad_name]  = link;
}

void Filter::set_input_format(const Format& f, const std::string& pad_name)
{
    if (input_pads_.find(pad_name) == input_pads_.end())
        throw dsp_error(Errc::pad_unknown);
    input_pads_[pad_name].format = f;
}

void Filter::set_output_format(const Format& f, const std::string& pad_name)
{
    if (output_pads_.find(pad_name) == output_pads_.end())
        throw dsp_error(Errc::pad_unknown);
    output_pads_[pad_name].format = f;
}

const Format& Filter::get_input_format(const std::string& pad_name)
{
    if (input_pads_.find(pad_name) == input_pads_.end())
        throw dsp_error(Errc::pad_unknown);
    return input_pads_[pad_name].format;
}

const Format& Filter::get_output_format(const std::string& pad_name)
{
    if (output_pads_.find(pad_name) == output_pads_.end())
        throw dsp_error(Errc::pad_unknown);
    return output_pads_[pad_name].format;
}

void Filter::update_stats(std::chrono::duration<double>& duration)
{
    stats_.n_execs++;
    stats_.durations.push_back(duration);
}

void Filter::reset_stats()
{
    stats_.n_execs = 0;
    stats_.durations.clear();
}

} /* namespace dsp */
