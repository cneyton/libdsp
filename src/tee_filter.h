#ifndef TEE_FILTER_H
#define TEE_FILTER_H

#include "filter.h"
#include "link.h"

namespace filter
{

template<typename T, arma::uword N>
class tee: public Filter
{
public:
    tee(common::Logger logger): Log(logger), Filter(logger, "tee") {}
    virtual ~tee() {}

    virtual int activate()
    {
        log_debug(logger_, "filter {} activated", this->name_);
        auto input  = dynamic_cast<Link<T>*>(inputs_.at(0));
        if (input->empty()) return 0;

        int ret;
        auto in_chunk = std::make_shared<Chunk<T>>();
        ret = input->front(in_chunk);
        common_die_zero(logger_, ret, -1, "failed to get front");
        ret = input->pop();
        common_die_zero(logger_, ret, -2, "failed to pop link");
        for (arma::uword i = 0; i < N; ++i) {
            auto output = dynamic_cast<Link<T>*>(outputs_.at(i));
            output->push(in_chunk);
        }
        return 1;
    }
private:
};

} /* namespace filter */

#endif /* TEE_FILTER_H */
