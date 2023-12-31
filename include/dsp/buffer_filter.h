#pragma once

#include <memory>
#include <deque>

#include "filter.h"
#include "link.h"

namespace dsp::filter {

/**
 * @brief Concatenate input chunks into larger output chunks.
 *
 * Shared pointers to the input chunks are stored inside an internal queue until
 * enough are present. The data is then copied to the output chunk and the queue
 * is cleared. The remaining data is stored inside an internal chunk for the next
 * output.
 * NB: fmt_in.n_rows must be < fmt_out.n_rows (format negotiation will fail otherwise)
 *
 * @tparam T Type of data processed by the filter.
 */
template<typename T>
class Buffer: public Filter
{
public:
    Buffer(common::Logger logger, std::string_view name = "buffer"):
        Filter(logger, name)
    {
        Pad in  {.name = "in" , .format = Format()};
        Pad out {.name = "out", .format = Format()};
        input_pads_.insert({in.name, in});
        output_pads_.insert({out.name, out});
    }

    int activate() override
    {
        log_debug(logger_, "{} filter activated", name_);

        auto input    = dynamic_cast<Link<T>*>(inputs_.at("in"));
        auto output   = dynamic_cast<Link<T>*>(outputs_.at("out"));
        auto chunk_in = std::make_shared<Chunk<T>>();

        if (!input->pop(chunk_in)) {
            if (input->eof())
                output->eof_reached();
            return 0;
        }
        chunk_queue_.push_back(chunk_in);

        const auto fmt_in  = input->format();
        const auto fmt_out = output->format();

        i_++;
        if (n_intern_ + fmt_in.n_rows * i_ < fmt_out.n_rows)
            return 0;

        i_ = 0;
        arma::uword timestamp = chunk_queue_.front()->timestamp;
        arma::uword t_intern  = n_intern_ * chunk_in->sample_period;
        if (t_intern > timestamp) {
            log_error(logger_, "invalid timestamp");
        } else {
            timestamp -= t_intern;
        }
        auto chunk_out = std::make_shared<Chunk<T>>(timestamp, chunk_in->sample_period, fmt_out);

        // first copy the internal chunk
        if (n_intern_ != 0) {
            chunk_out->rows(0, n_intern_ - 1) = chunk_intern_.rows(0, n_intern_ - 1);
        }
        arma::uword beg = n_intern_;
        arma::uword end = beg;
        n_intern_ = 0;
        for (auto it = chunk_queue_.cbegin(); it < chunk_queue_.cend(); ++it) {
            end += (*it)->n_rows;
            if (end > fmt_out.n_rows) {
                n_intern_ = end - fmt_out.n_rows;
                end = fmt_in.n_rows - n_intern_;
                chunk_out->rows(beg, fmt_out.n_rows - 1) = (*it)->rows(0, end - 1);
                chunk_intern_.rows(0, n_intern_ - 1) = (*it)->rows(end, fmt_in.n_rows - 1);
            } else {
                chunk_out->rows(beg, end - 1) = **it;
                beg = end;
            }
        }
        chunk_queue_.clear();

        output->push(chunk_out);
        return 1;
    }

    void reset() override
    {
        i_ = 0;
        n_intern_ = 0;
        chunk_queue_.clear();
    }

    Contract negotiate_format() override
    {
        auto fmt_in  = input_pads_["in"].format;
        auto fmt_out = output_pads_["out"].format;

        if (fmt_in.n_cols   != fmt_out.n_cols ||
            fmt_in.n_slices != fmt_out.n_slices ||
            fmt_in.n_rows   >= fmt_out.n_rows) {
            return Contract::unsupported_format;
        }

        chunk_intern_.set_size(fmt_in.n_rows, fmt_in.n_cols, fmt_in.n_slices);

        return Contract::supported_format;
    }

private:
    arma::uword i_ = 0;
    arma::uword n_intern_ = 0;
    Chunk<T>    chunk_intern_;
    std::deque<std::shared_ptr<Chunk<T>>> chunk_queue_;
};

} /* namespace dsp::filter */
