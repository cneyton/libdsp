#ifndef BUFFER_FILTER_H
#define BUFFER_FILTER_H

#include <memory>
#include <deque>

#include "filter.h"
#include "link.h"

namespace filter
{

template<typename T>
class buffer: public Filter
{
public:
    buffer(common::Logger logger): Log(logger), Filter(logger, "buffer") {}
    virtual ~buffer() {}

    virtual int activate()
    {
        log_debug(logger_, "{} filter activated", this->name_);

        auto input = dynamic_cast<Link<T>*>(inputs_.at(0));

        if (input->empty()) return 0;
        auto chunk_in = input->front();
        input->pop();

        chunk_queue_.push_back(chunk_in);

        auto output  = dynamic_cast<Link<T>*>(outputs_.at(0));
        auto fmt_in  = input->get_format();
        auto fmt_out = output->get_format();

        i_++;
        if (n_intern_ + fmt_in.n_rows * i_ < fmt_out.n_rows)
            return 0;

        i_ = 0;
        auto chunk_out = std::make_shared<Chunk<T>>(fmt_out);
        /* TODO: remove when fmt negociation is done <20-03-20, cneyton> */
        chunk_intern_.set_size(fmt_in);
        // first copy the internal chunk
        if (n_intern_ != 0) {
            chunk_out->rows(0, n_intern_ - 1) = chunk_intern_.rows(0, n_intern_ - 1);
        }
        arma::uword start = n_intern_;
        arma::uword end   = start;
        n_intern_ = 0;
        for (auto it = chunk_queue_.cbegin(); it < chunk_queue_.cend(); ++it) {
            end += (*it)->n_rows;
            if (end > fmt_out.n_rows) {
                n_intern_ = end - fmt_out.n_rows;
                end = fmt_in.n_rows - n_intern_;
                chunk_out->rows(start, fmt_out.n_rows - 1) = (*it)->rows(0, end - 1);
                chunk_intern_.rows(0, n_intern_ - 1) = (*it)->rows(end, fmt_in.n_rows - 1);
            } else {
                chunk_out->rows(start, end - 1) = **it;
                start = end;
            }
        }
        chunk_queue_.clear();

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

        if (fmt_in.n_rows >= fmt_out.n_rows)
            return 0;

        chunk_intern_.set_size(fmt_in);

        return 1;
    }

private:
    arma::uword i_ = 0;
    arma::uword n_intern_ = 0;
    Chunk<T>    chunk_intern_;
    std::deque<std::shared_ptr<Chunk<T>>> chunk_queue_;
};

} /* namespace filter */

#endif /* BUFFER_FILTER_H */
