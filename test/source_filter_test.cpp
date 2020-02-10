#include "common/log.h"
#include "common/data.h"

#include "pipeline.h"
#include "source_filter.h"
#include "sink_filter.h"

#include "spdlog/sinks/stdout_color_sinks.h"

int main()
{
    int ret;
    common::Logger logger(spdlog::stdout_color_mt("dsp"));

    common::data::Handler data_handler(logger);

    Pipeline pipeline(logger);

    auto source_filter = new filter::Source(logger, &data_handler, common::data::type::us);
    source_filter->set_format(sizeof(arma::cx_float), 5, 2);
    source_filter->set_nb_frames(2);
    pipeline.add_filter(source_filter);

    auto sink_filter = new filter::Sink(logger);
    pipeline.add_filter(sink_filter);

    pipeline.link(source_filter, sink_filter);

    ret = pipeline.run();
    common_die_zero(logger, ret, -1, "pipeline run error");

    return 0;
}
