#include "common/log.h"
#include "common/data.h"
#include <armadillo>
#include <memory>

#include "chunk.h"
#include "link.h"
#include "source_filter.h"
#include "sink_filter.h"
#include "spdlog/sinks/stdout_color_sinks.h"


int main()
{
    common::Logger logger(spdlog::stdout_color_mt("dsp"));

    common::data::Handler data_handler(logger);

    std::vector<Filter*> filters;
    auto source_filter = new filter::Source(logger, &data_handler, common::data::type::us);
    source_filter->set_format(sizeof(arma::cx_float), 5, 2);
    source_filter->set_nb_frames(2);
    filters.push_back(source_filter);

    auto sink_filter = new filter::Sink(logger);
    filters.push_back(sink_filter);

    Link link(logger, source_filter, sink_filter);

    for(auto& filter: filters)
    {
        filter->activate();
    }

    return 0;
}
