#include <thread>
#include <chrono>
#include <random>

#include "common/log.h"
#include "common/data.h"

#include "fhr_filter.h"
#include "pipeline.h"
#include "source_filter.h"
#include "sink_filter.h"

#include "spdlog/sinks/stdout_color_sinks.h"
common::Logger logger(spdlog::stdout_color_mt("dsp"));

using iT = int16_t;
using oT = double;

constexpr uint16_t nb_samples = 36;
constexpr uint16_t nb_slots   = 7;
constexpr uint16_t nb_frames  = 1;
constexpr size_t   elt_size   = nb_samples * nb_slots * sizeof(iT);
constexpr uint     nb_tot_frames = 1000;

// fhr filter params
constexpr arma::uword radius   = 7;
constexpr arma::uword fdperseg = 133;
constexpr arma::uword fdskip   = 8;
constexpr filter::fhr<oT, oT, oT>::period_range range{0.0, 5.0};

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
        p.push(common::data::type::oxy, buf);
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
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

    auto source_filter = new filter::source<iT, oT>(logger, &data_handler, common::data::type::oxy);
    source_filter->set_chunk_size(nb_frames, nb_samples, nb_slots);
    pipeline.add_filter(std::unique_ptr<Filter>(source_filter));

    auto sink_filter_0 = new filter::sink<oT>(logger);
    pipeline.add_filter(std::unique_ptr<Filter>(sink_filter_0));

    auto sink_filter_1 = new filter::sink<oT>(logger);
    pipeline.add_filter(std::unique_ptr<Filter>(sink_filter_1));

    auto fhr_filter = new filter::fhr<oT, oT, oT>(logger, fdperseg, fdskip, radius, range);
    pipeline.add_filter(std::unique_ptr<Filter>(fhr_filter));

    std::cout << "Filter params:\n"
              << "   fdperseg: " << fdperseg << "\n"
              << "   fdskip:   " << fdskip   << "\n"
              << "   radius:   " << radius   << "\n"
              << "   range:    (" << range.min << "," << range.max <<  ")\n"
              << "------------------------------\n";

    arma::SizeCube format(nb_frames, nb_samples, nb_slots);
    pipeline.link<oT>(source_filter, fhr_filter, format);
    pipeline.link<oT>(fhr_filter, sink_filter_0, format);
    pipeline.link<oT>(fhr_filter, sink_filter_1, format);

    data_handler.reinit_queue(common::data::type::oxy, elt_size, 1000);

    std::thread pipeline_th(pipeline_th_func, std::ref(pipeline));
    std::thread producer_th(producer_th_func, std::ref(producer));

    producer_th.join();
    pipeline.stop();
    pipeline_th.join();

    pipeline.print_stats();

    return 0;
}

