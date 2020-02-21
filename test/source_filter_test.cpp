#include <memory>
#include <thread>
#include <chrono>
#include <random>

#include "common/log.h"
#include "common/data.h"

#include "pipeline.h"
#include "source_filter.h"
#include "sink_filter.h"

#include "spdlog/common.h"
#include "spdlog/sinks/stdout_color_sinks.h"
common::Logger logger(spdlog::stdout_color_mt("dsp"));

using iT = int16_t;//filter::iq<int16_t>;
using oT = arma::cx_double;

uint16_t nb_samples = 10;
uint16_t nb_slots   = 1;
uint16_t nb_frames  = 2;
size_t   elt_size   = nb_samples * nb_slots * sizeof(iT);

void producer_th_func(common::data::Producer& p)
{
    std::random_device rd;
    std::uniform_int_distribution dist(0, 254);
    for (uint i=0; i < 20; i++) {
        common::data::ByteBuffer buf(elt_size, 1);
        //std::transform(buf.begin(), buf.end(), buf.begin(),
                       //[&](int x){return x * static_cast<uint8_t>(dist(rd));});
        p.push(common::data::type::us, buf);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void pipeline_th_func(Pipeline& pipeline)
{
    int ret;
    while (1) {
        ret = pipeline.run();
        common_die_zero_void(logger, ret, "pipeline run error");
        if (ret == 0) {
            return;
        }
    }
}

/* TODO: we should test different type of input/output <20-02-20, cneyton> */
int main()
{
    common::data::Handler data_handler(logger);
    common::data::Producer producer(logger, &data_handler);
    logger->set_level(spdlog::level::debug);

    Pipeline pipeline(logger);

    auto source_filter = new filter::source<iT, oT>(logger, &data_handler,
                                                             common::data::type::us);
    source_filter->set_chunk_size(nb_frames, nb_samples, nb_slots);
    source_filter->set_verbose();
    pipeline.add_filter(std::unique_ptr<Filter>(source_filter));

    auto sink_filter = new filter::sink<oT>(logger);
    sink_filter->set_verbose();
    pipeline.add_filter(std::unique_ptr<Filter>(sink_filter));

    pipeline.link<oT>(source_filter, sink_filter);

    data_handler.reinit_queue(common::data::type::us, elt_size, 100);

    std::thread pipeline_th(pipeline_th_func, std::ref(pipeline));
    std::thread producer_th(producer_th_func, std::ref(producer));

    pipeline_th.join();
    producer_th.join();

    return 0;
}
