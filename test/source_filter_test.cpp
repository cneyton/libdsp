#include <thread>
#include <chrono>

#include "common/log.h"
#include "common/data.h"

#include "pipeline.h"
#include "source_filter.h"
#include "sink_filter.h"

#include "spdlog/sinks/stdout_color_sinks.h"
common::Logger logger(spdlog::stdout_color_mt("dsp"));

size_t elt_size = 40;

void producer_th_func(common::data::Producer& p)
{
    common::data::ByteBuffer buf
        = {1,  10, 2,  0, 3,  0, 4,  0, 5,  0, 6,  0, 7,  0, 8,  0, 9,  0, 10, 0,
           11, 10, 12, 0, 13, 0, 14, 0, 15, 0, 16, 0, 17, 0, 18, 0, 19, 0, 20, 0};

    while (1) {
        p.push(common::data::type::us, buf);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void pipeline_th_func(Pipeline& pipeline)
{
    int ret;
    while (1) {
        ret = pipeline.run();
        common_die_zero_void(logger, ret, "pipeline run error");
        if (ret == 0) {
        }
    }
}

int main()
{
    common::data::Handler data_handler(logger);
    common::data::Producer producer(logger, &data_handler);

    Pipeline pipeline(logger);

    auto source_filter = new filter::Source(logger, &data_handler, common::data::type::us);
    source_filter->set_format(sizeof(arma::cx_float), 5, 2);
    source_filter->set_nb_frames(2);
    pipeline.add_filter(source_filter);

    auto sink_filter = new filter::Sink(logger);
    pipeline.add_filter(sink_filter);

    pipeline.link(source_filter, sink_filter);

    data_handler.reinit_queue(common::data::type::us, elt_size, 100);

    std::thread pipeline_th(pipeline_th_func, std::ref(pipeline));
    std::thread producer_th(producer_th_func, std::ref(producer));

    pipeline_th.join();
    producer_th.join();

    return 0;
}
