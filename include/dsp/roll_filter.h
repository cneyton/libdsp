#pragma once

#include <memory>
#include <deque>

#include "filter.h"
#include "link.h"

namespace dsp::filter {

/**
 * Roll filter: concatenate input chunks into larger output chunks with overlap
 * Shared pointers to the input chunks are stored inside an internal queue until
 * enough are present. The data is then copied to the output chunk. 'skip' chunk
 * are then cleared.
 * NB: fmt_out.n_rows must be a multiple of fmt_out.n_rows (format negotiation
 * will fail otherwise)
 */
template<typename T>
class roll: public Filter
{
public:
    roll(common::Logger logger, arma::uword skip):
        Log(logger), Filter(logger, "roll"), skip_(skip) {}
    ~roll() = default;

    int activate() override
    {
        log_debug(logger_, "{} filter activated", name_);

        auto input    = dynamic_cast<Link<T>*>(inputs_.at(0));
        auto output   = dynamic_cast<Link<T>*>(outputs_.at(0));
        auto chunk_in = std::make_shared<Chunk<T>>();

        int ret;
        ret = input->pop(chunk_in);
        common_die_zero(logger_, ret, -1, "failed to pop chunk");
        if (!ret) {
            if (input->eof())
                output->eof_reached();
            return 0;
        }

        chunk_queue_.push_back(chunk_in);
        i_++;

        auto fmt_in  = input->get_format();
        auto fmt_out = output->get_format();

        /* TODO: replace by queue_size_ when fmt negotatiation is done <23-03-20, cneyton> */
        arma::uword queue_size = fmt_out.n_rows / fmt_in.n_rows;

        // first filling of the queue
        if (i_ < queue_size)
            return 0;

        if ((i_ - queue_size) % skip_ != 0) {
            chunk_queue_.pop_front();
            return 0;
        }

        auto chunk_out = std::make_shared<Chunk<T>>(fmt_out);
        for (arma::uword i = 0; i < queue_size; ++i) {
            chunk_out->rows(i * fmt_in.n_rows, (i+1) * fmt_in.n_rows - 1) = *(chunk_queue_[i]);
        }
        chunk_queue_.pop_front();
        output->push(chunk_out);
        return 1;
    }

    int negotiate_fmt() // override
    {
        auto input   = dynamic_cast<Link<T>*>(inputs_.at(0));
        auto output  = dynamic_cast<Link<T>*>(outputs_.at(0));
        auto fmt_in  = input->get_format();
        auto fmt_out = output->get_format();

        if (fmt_in.n_cols != fmt_out.n_cols || fmt_in.n_slices != fmt_out.n_slices)
            return 0;

        if (fmt_out.n_rows % fmt_in.n_rows != 0)
            return 0;

        queue_size_ = fmt_out.n_rows / fmt_in.n_rows;

        return 1;
    }

private:
    arma::uword skip_;
    arma::uword i_ = 0;
    std::deque<std::shared_ptr<Chunk<T>>> chunk_queue_;
    arma::uword queue_size_;
};

} /* namespace dsp::filter */
