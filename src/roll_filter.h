#ifndef ROLL_FILTER_H
#define ROLL_FILTER_H

#include <memory>
#include <deque>

#include "filter.h"
#include "link.h"

namespace filter
{

template<typename T>
class roll: public Filter
{
public:
    roll(common::Logger logger, arma::uword skip): Log(logger), Filter(logger, "roll"),
        skip_(skip) {}
    virtual ~roll() {}

    virtual int activate()
    {
        log_debug(logger_, "{} filter activated", this->name_);

        auto input = dynamic_cast<Link<T>*>(inputs_.at(0));
        if (input->empty()) return 0;

        auto chunk_in = input->front();
        input->pop();

        chunk_queue_.push_back(chunk_in);
        i_++;

        auto output  = dynamic_cast<Link<T>*>(outputs_.at(0));
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

    virtual int negotiate_fmt()
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

} /* namespace filter */

#endif /* end of include guard: ROLL_FILTER_H */

