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
    roll(common::Logger logger): Log(logger), Filter(logger, "roll") {}
    virtual ~roll() {}

    virtual int activate()
    {
        log_debug(logger_, "{} filter activated", this->name_);

        auto input = dynamic_cast<Link<T>*>(inputs_.at(0));
        if (input->empty()) return 0;

        int ret;
        auto in_chunk = std::make_shared<Chunk<T>>();
        ret = input->front(in_chunk);
        common_die_zero(logger_, ret, -1, "failed to get front");
        ret = input->pop();
        common_die_zero(logger_, ret, -2, "failed to pop link");

        chunk_queue_.push_back(in_chunk);

        auto output  = dynamic_cast<Link<T>*>(outputs_.at(0));
        auto in_fmt  = input->get_format();
        auto out_fmt = output->get_format();

        i_++;
        if (in_fmt.n_rows * i_ < out_fmt.n_rows)
            return 0;

        i_ = 0;
        auto out_chunk = std::make_shared<Chunk<T>>(out_fmt);
        arma::uword row_start = 0;
        arma::uword row_end   = 0;
        for (auto it = chunk_queue_.crbegin(); it < chunk_queue_.crend(); ++it) {
            row_end += (*it)->n_rows;
            out_chunk->rows(row_start, row_end - 1) = **it;
            row_start = row_end;
        }
        for (arma::uword i = 0; i < out_fmt.n_rows; ++i)
            chunk_queue_.pop_front();

        output->push(out_chunk);
        return 1;
    }

private:
    arma::uword i_ = 0;

    std::deque<std::shared_ptr<Chunk<T>>> chunk_queue_;
};

} /* namespace filter */

#endif /* ROLL_FILTER_H */
