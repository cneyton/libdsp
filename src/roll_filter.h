#ifndef ROLL_FILTER_H
#define ROLL_FILTER_H

#include "filter.h"
#include "link.h"
#include <memory>

namespace filter
{

template<typename T>
class roll: public Filter
{
public:
    roll(common::Logger logger, arma::SizeCube& format, arma::uword nskip):
        Log(logger), Filter(logger, "roll"), internal_chunk_(format), nskip_(nskip)
    {
    }
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

        auto size = arma::size(internal_chunk_);
        for (arma::uword i = 0; i < size.n_slices; ++i) {
            internal_chunk_.slice(i) = arma::shift(internal_chunk_.slice(i), nskip_);
            internal_chunk_.slice(i).rows(0, nskip_ - 1) = in_chunk->slice(i);
        }

        auto out_chunk = std::make_shared<Chunk<T>>(internal_chunk_);

        auto output = dynamic_cast<Link<T>*>(outputs_.at(0));
        output->push(out_chunk);

        return 1;
    }

private:
    Chunk<T>    internal_chunk_;
    arma::uword nskip_;
};

} /* namespace filter */

#endif /* ROLL_FILTER_H */
