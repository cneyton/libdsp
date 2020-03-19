#include <thread>
#include <chrono>
#include <random>

#include "common/log.h"
#include "common/data.h"

#include "tee_filter.h"
#include "pipeline.h"
#include "source_filter.h"
#include "sink_filter.h"

#include "spdlog/sinks/stdout_color_sinks.h"
common::Logger logger(spdlog::stdout_color_mt("dsp"));

using iT = double;
using oT = double;

constexpr uint16_t nb_samples = 36;
constexpr uint16_t nb_slots   = 7;
constexpr uint16_t nb_frames  = 1;
constexpr size_t   elt_size   = nb_samples * nb_slots * sizeof(iT);
constexpr uint     nb_tot_frames = 10000;
constexpr uint     period = 1;
constexpr uint     N = 2;

class Handler: public common::data::Handler
{
public:
    Handler(common::Logger logger, Pipeline * pipeline):
        common::data::Handler(logger), pipeline_(pipeline) {}
    virtual ~Handler() {}

    virtual int data_pushed()
    {
        pipeline_->resume();
        return 0;
    }

private:
    Pipeline * pipeline_;
};

void producer_th_func(common::data::Producer& p)
{
    std::random_device rd;
    std::uniform_int_distribution dist(0, 254);
    for (uint i = 0; i < nb_tot_frames; i++) {
        common::data::ByteBuffer buf(elt_size, 1);
        std::transform(buf.begin(), buf.end(), buf.begin(),
                       [&](int x){return x * static_cast<uint8_t>(dist(rd));});
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
    std::cout << "Input:\n"
              << "   type: " << typeid(iT).name() << "\n"
              << "   chunk size: (" << nb_frames  << "," << nb_samples << "," << nb_slots << ")\n"
              << "   nb frames: " << nb_tot_frames << "\n"
              << "------------------------------\n";

    logger->set_level(spdlog::level::info);

    Pipeline pipeline(logger);
    Handler data_handler(logger, &pipeline);
    common::data::Producer producer(logger, &data_handler);

    auto fmt = arma::SizeCube(nb_frames, nb_samples, nb_slots);
    auto source_filter = new filter::source<iT, oT>(logger, &data_handler, common::data::type::us);
    source_filter->set_chunk_size(nb_frames, nb_samples, nb_slots);
    pipeline.add_filter(std::unique_ptr<Filter>(source_filter));

    auto sink_filter_1 = new filter::sink<double>(logger);
    pipeline.add_filter(std::unique_ptr<Filter>(sink_filter_1));

    auto sink_filter_2 = new filter::sink<double>(logger);
    pipeline.add_filter(std::unique_ptr<Filter>(sink_filter_2));

    auto tee_filter = new filter::tee<oT, N>(logger);
    pipeline.add_filter(std::unique_ptr<Filter>(tee_filter));

    std::cout << "Filter params:\n"
              << "   N: " << N << "\n"
              << "------------------------------\n";

    pipeline.link<iT>(source_filter, tee_filter, fmt);
    pipeline.link<oT>(tee_filter, sink_filter_1, fmt);
    pipeline.link<oT>(tee_filter, sink_filter_2, fmt);

    data_handler.reinit_queue(common::data::type::us, elt_size, 1000);

    std::thread pipeline_th(pipeline_th_func, std::ref(pipeline));
    std::thread producer_th(producer_th_func, std::ref(producer));

    producer_th.join();
    pipeline.stop();
    pipeline_th.join();

    pipeline.print_stats();

    return 0;
}

