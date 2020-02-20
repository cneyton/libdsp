#include <memory>
#include <thread>
#include <chrono>
#include <random>

#include "common/log.h"
#include "common/data.h"

#include "pipeline.h"
#include "source_filter.h"
#include "sink_filter.h"

#include "spdlog/sinks/stdout_color_sinks.h"
common::Logger logger(spdlog::stdout_color_mt("dsp"));

uint16_t nb_samples = 10;
uint16_t nb_slots   = 1;
uint16_t nb_frames  = 2;
size_t   elt_size   = nb_samples * nb_slots * 2;

void producer_th_func(common::data::Producer& p)
{
    std::random_device rd;
    std::uniform_int_distribution dist(0, 254);
    while (1) {
        common::data::ByteBuffer buf(elt_size, 1);
        std::transform(buf.begin(), buf.end(), buf.begin(),
                       [&](int x){return x * static_cast<uint8_t>(dist(rd));});
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

/* TODO: we should test different type of input/output <20-02-20, cneyton> */
int main()
{
    common::data::Handler data_handler(logger);
    common::data::Producer producer(logger, &data_handler);

    Pipeline pipeline(logger);

    //using iT = filter::iq<int16_t>;
    auto source_filter = new filter::source<int16_t, double>(logger, &data_handler,
                                                             common::data::type::us);
    source_filter->set_chunk_size(nb_frames, nb_samples, nb_slots);
    pipeline.add_filter(std::unique_ptr<Filter>(source_filter));

    auto sink_filter = new filter::sink<double>(logger);
    sink_filter->set_verbose();
    pipeline.add_filter(std::unique_ptr<Filter>(sink_filter));

    pipeline.link<double>(source_filter, sink_filter);

    data_handler.reinit_queue(common::data::type::us, elt_size, 100);

    std::thread pipeline_th(pipeline_th_func, std::ref(pipeline));
    std::thread producer_th(producer_th_func, std::ref(producer));

    pipeline_th.join();
    producer_th.join();

    return 0;
}
