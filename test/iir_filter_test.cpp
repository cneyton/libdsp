#include <memory>
#include <thread>
#include <chrono>
#include <random>

#include "common/log.h"
#include "common/data.h"

#include "iir_filter.h"
#include "pipeline.h"
#include "source_filter.h"
#include "sink_filter.h"

#include "spdlog/sinks/stdout_color_sinks.h"
common::Logger logger(spdlog::stdout_color_mt("dsp"));

using iT = filter::iq<int16_t>;
using oT = arma::cx_double;

uint16_t nb_samples = 36;
uint16_t nb_slots   = 7;
uint16_t nb_frames  = 5;
size_t   elt_size   = nb_samples * nb_slots * sizeof(iT);

void producer_th_func(common::data::Producer& p)
{
    std::random_device rd;
    std::uniform_int_distribution dist(0, 254);
    while (1) {
        common::data::ByteBuffer buf(elt_size, 1);
        std::transform(buf.begin(), buf.end(), buf.begin(),
                       [&](int x){return x * static_cast<uint8_t>(dist(rd));});
        p.push(common::data::type::us, buf);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void pipeline_th_func(Pipeline& pipeline)
{
    int ret;
    while (1) {
        ret = pipeline.run_once();
        common_die_zero_void(logger, ret, "pipeline run error");
        if (ret == 0) {
        }
    }
}

int main()
{
    logger->set_level(spdlog::level::info);
    common::data::Handler data_handler(logger);
    common::data::Producer producer(logger, &data_handler);

    Pipeline pipeline(logger);

    auto source_filter = new filter::source<iT, oT>(logger, &data_handler,
                                                             common::data::type::us);
    source_filter->set_chunk_size(nb_frames, nb_samples, nb_slots);
    pipeline.add_filter(std::unique_ptr<Filter>(source_filter));

    auto sink_filter = new filter::sink<oT>(logger);
    pipeline.add_filter(std::unique_ptr<Filter>(sink_filter));

    auto b = arma::vec(3);
    auto a = arma::vec(3);
    b << 1.7 << 2.8 << 3.6 << arma::endr;
    a << 1   << 5.4 << 1.4 << arma::endr;

    auto iir_filter = new filter::iir<arma::cx_double>(logger, nb_slots * nb_samples, b, a);
    pipeline.add_filter(std::unique_ptr<Filter>(iir_filter));

    pipeline.link<oT>(source_filter, iir_filter);
    pipeline.link<oT>(iir_filter, sink_filter);

    data_handler.reinit_queue(common::data::type::us, elt_size, 100);

    std::thread pipeline_th(pipeline_th_func, std::ref(pipeline));
    std::thread producer_th(producer_th_func, std::ref(producer));

    pipeline_th.join();
    producer_th.join();

    pipeline.print_stats();

    return 0;
}
