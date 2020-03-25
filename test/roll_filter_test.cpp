#include <thread>
#include <chrono>
#include <random>

#include "common/log.h"
#include "common/data.h"

#include "roll_filter.h"
#include "pipeline.h"
#include "source_filter.h"
#include "sink_filter.h"

#include "spdlog/sinks/stdout_color_sinks.h"
common::Logger logger(spdlog::stdout_color_mt("dsp"));

// input params
using iT = uint8_t;
using oT = double;

constexpr uint16_t nb_samples = 5;
constexpr uint16_t nb_slots   = 1;
constexpr uint16_t nb_frames  = 6;
constexpr uint16_t nb_frames_out = nb_frames * 3;
constexpr size_t   elt_size   = nb_samples * nb_slots * sizeof(iT);
constexpr uint     nb_tot_frames = 60;
constexpr uint     period = 1;

// Filter params
constexpr uint skip = 4;

class Handler: public common::data::Handler
{
public:
    Handler(common::Logger logger, Pipeline * pipeline):
        common::data::Handler(logger), pipeline_(pipeline) {}
    virtual ~Handler() {}

    virtual int data_pushed()
    {
        pipeline_->wakeup();
        return 0;
    }

private:
    Pipeline * pipeline_;
};

void producer_th_func(common::data::Producer& p)
{
    for (uint i = 0; i < nb_tot_frames; i++) {
        common::data::ByteBuffer buf(elt_size, i);
        p.push(common::data::type::us, buf);
        std::this_thread::sleep_for(std::chrono::milliseconds(period));
    }
}

void pipeline_th_func(Pipeline& pipeline)
{
    pipeline.run();
}

int main()
{
    logger->set_level(spdlog::level::info);

    Pipeline pipeline(logger);
    Handler data_handler(logger, &pipeline);
    common::data::Producer producer(logger, &data_handler);

    arma::SizeCube fmt_in = arma::SizeCube(nb_frames, nb_samples, nb_slots);
    auto source_filter = new filter::source<iT, oT>(logger, &data_handler, common::data::type::us);
    source_filter->set_chunk_size(nb_frames, nb_samples, nb_slots);
    pipeline.add_filter(std::unique_ptr<Filter>(source_filter));

    arma::SizeCube fmt_out = arma::SizeCube(nb_frames_out, nb_samples, nb_slots);
    auto sink_filter = new filter::sink<oT>(logger);
    sink_filter->set_verbose();
    pipeline.add_filter(std::unique_ptr<Filter>(sink_filter));

    auto roll_filter = new filter::roll<oT>(logger, skip);
    pipeline.add_filter(std::unique_ptr<Filter>(roll_filter));

    std::cout << "Filter params:\n"
              << "  skip: " << skip << "\n"
              << "------------------------------\n";

    pipeline.link<oT>(source_filter, roll_filter, fmt_in);
    pipeline.link<oT>(roll_filter, sink_filter, fmt_out);

    data_handler.reinit_queue(common::data::type::us, elt_size, 1000);

    std::thread pipeline_th(pipeline_th_func, std::ref(pipeline));
    std::thread producer_th(producer_th_func, std::ref(producer));

    producer_th.join();
    pipeline.stop();
    pipeline_th.join();

    pipeline.print_stats();

    return 0;
}
