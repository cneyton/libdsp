#pragma once

#include <memory>
#include <deque>

#include "filter.h"
#include "link.h"

namespace dsp::filter {

/**
 * Concatenate 'n_per_seg' input chunks into larger output chunks with overlap.
 * Shared pointers to the input chunks are stored inside an internal queue until
 * enough are present. The data is then copied to the output chunk. 'n_skip' chunk
 * are then cleared.
 *
 * @tparam T chunk type
 */
template<typename T>
class Roll: public Filter
{
public:
    Roll(common::Logger logger, std::string_view name,
         arma::uword n_per_seg, arma::uword n_skip):
        Filter(logger, name), n_skip_(n_skip), n_per_seg_(n_per_seg)
    {
        Pad in  {.name = "in" , .format = Format()};
        Pad out {.name = "out", .format = Format()};
        input_pads_.insert({in.name, in});
        output_pads_.insert({out.name, out});
    }

    Roll(common::Logger logger, arma::uword n_per_seg, arma::uword n_skip):
        Roll(logger, "roll", n_per_seg, n_skip)
    {
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
        i_++;

        const auto fmt_in  = input->format();
        const auto fmt_out = output->format();

        // first filling of the queue
        if (i_ < n_per_seg_)
            return 0;

        if ((i_ - n_per_seg_) % n_skip_ != 0) {
            chunk_queue_.pop_front();
            return 0;
        }

        auto chunk_out = std::make_shared<Chunk<T>>(chunk_queue_.front()->timestamp,
                                                    chunk_queue_.front()->sample_period,
                                                    fmt_out);
        for (arma::uword i = 0; i < n_per_seg_; ++i) {
            chunk_out->rows(i * fmt_in.n_rows, (i+1) * fmt_in.n_rows - 1) = *(chunk_queue_[i]);
        }
        chunk_queue_.pop_front();
        output->push(chunk_out);
        return 1;
    }

    void reset() override
    {
        i_ = 0;
        chunk_queue_.clear();
    }

    Contract negotiate_format() override
    {
        auto fmt_in  = input_pads_["in"].format;
        auto fmt_out = output_pads_["out"].format;

        if ((fmt_in.n_cols   != fmt_out.n_cols) ||
            (fmt_in.n_slices != fmt_out.n_slices) ||
            (fmt_in.n_rows * n_per_seg_ != fmt_out.n_rows)) {
            return Contract::unsupported_format;
        }

        return Contract::supported_format;
    }

private:
    arma::uword n_skip_;
    arma::uword i_ = 0;
    std::deque<std::shared_ptr<Chunk<T>>> chunk_queue_;
    arma::uword n_per_seg_;
};

} /* namespace dsp::filter */
