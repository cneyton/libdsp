#ifndef IIR_FILTER_H
#define IIR_FILTER_H

#include <vector>

#include "common/log.h"

#include "filter.h"
#include "sigpack.h"

namespace filter
{

template<typename T>
class IIR: public Filter
{
public:
    IIR(common::Logger logger);
    IIR(common::Logger logger, uint16_t nb_filters);
    IIR(common::Logger logger, uint16_t nb_filters, const arma::vec& b, const arma::vec& a);
    virtual ~IIR();

    virtual int activate();

private:
    std::vector<sp::IIR_filt<T, double, T>> filters_;
};

} /* namespace filter */

#include "iir_filter.ipp"

#endif /* IIR_FILTER_H */
