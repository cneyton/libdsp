#include <thread>
#include <chrono>

#include "common/log.h"
#include "common/data.h"

#include "dsp/fft_filter.h"
#include "dsp/pipeline.h"
#include "dsp/source_filter.h"
#include "dsp/sink_filter.h"

#include "spdlog/sinks/stdout_color_sinks.h"
common::Logger logger(spdlog::stdout_color_mt("dsp"));

uint16_t nb_samples = 36;
uint16_t nb_slots   = 7;
uint16_t nb_frames  = 128;
size_t elt_size = nb_samples * nb_slots * 4;

void producer_th_func(common::data::Producer& p)
{
    while (1) {
        common::data::ByteBuffer buf(elt_size, 1);
        p.push(common::data::type::us, buf);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
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
    logger->set_level(spdlog::level::info);
    common::data::Handler data_handler(logger);
    common::data::Producer producer(logger, &data_handler);

    Pipeline pipeline(logger);

    auto source_filter = new filter::Source(logger, &data_handler, common::data::type::us);
    source_filter->set_format(sizeof(arma::cx_float), nb_samples, nb_slots);
    source_filter->set_nb_frames(nb_frames);
    pipeline.add_filter(source_filter);

    auto sink_filter = new filter::Sink(logger);
    pipeline.add_filter(sink_filter);

    auto fft_filter = new filter::FFT<arma::cx_double>(logger, nb_frames);
    pipeline.add_filter(fft_filter);

    pipeline.link(source_filter, fft_filter);
    pipeline.link(fft_filter, sink_filter);

    data_handler.reinit_queue(common::data::type::us, elt_size, 1000);

    std::thread pipeline_th(pipeline_th_func, std::ref(pipeline));
    std::thread producer_th(producer_th_func, std::ref(producer));

    pipeline_th.join();
    producer_th.join();

    return 0;
}
