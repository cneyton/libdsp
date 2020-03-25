#include <thread>
#include <chrono>
#include <random>

#include "common/log.h"
#include "common/data.h"

#include "source_filter.h"
#include "iir_filter.h"
#include "roll_filter.h"
#include "buffer_filter.h"
#include "fhr_filter.h"
#include "sink_filter.h"
#include "pipeline.h"
#include "spdlog/sinks/stdout_color_sinks.h"

common::Logger logger(spdlog::stdout_color_mt("dsp"));

using iT = uint8_t;
using oT = double;

constexpr uint16_t nb_samples = 36;
constexpr uint16_t nb_slots   = 7;
constexpr uint16_t nb_frames  = 1;
constexpr size_t   elt_size   = nb_samples * nb_slots * sizeof(iT);
constexpr uint     nb_tot_frames = 10000;
constexpr uint     period = 30;

// filter params
constexpr arma::uword fdperseg   = 133;
constexpr arma::uword fdskip     = 8;
constexpr arma::uword radius     = 7;
constexpr arma::uword period_max = 67;
constexpr double      threshold  = 0.6;

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
    logger->set_level(spdlog::level::info);

    Pipeline pipeline(logger);
    Handler data_handler(logger, &pipeline);
    common::data::Producer producer(logger, &data_handler);

    arma::SizeCube fmt_source(nb_frames, nb_samples, nb_slots);
    auto source_filter = new filter::source<iT, oT>(logger, &data_handler, common::data::type::us);
    source_filter->set_chunk_size(nb_frames, nb_samples, nb_slots);
    pipeline.add_filter(std::unique_ptr<Filter>(source_filter));

    arma::SizeCube fmt_buffer(fdskip, nb_samples, nb_slots);
    auto buffer_filter = new filter::buffer<oT>(logger);
    pipeline.add_filter(std::unique_ptr<Filter>(buffer_filter));

    auto b = arma::vec(3);
    auto a = arma::vec(3);
    b << 1.7 << 2.8 << 3.6 << arma::endr;
    a << 1.0 << 5.4 << 1.4 << arma::endr;

    auto iir_filter = new filter::iir<oT, double>(logger, nb_slots * nb_samples, b, a);
    pipeline.add_filter(std::unique_ptr<Filter>(iir_filter));

    arma::SizeCube fmt_roll(fdperseg, nb_samples, nb_slots);
    auto roll_filter = new filter::roll<oT>(logger, fdskip);
    pipeline.add_filter(std::unique_ptr<Filter>(roll_filter));

    arma::SizeCube fmt_fhr(1, nb_samples, nb_slots);
    auto fhr_filter = new filter::fhr<oT, oT, oT>(logger, radius, period_max, threshold);
    pipeline.add_filter(std::unique_ptr<Filter>(fhr_filter));

    auto sink_filter_0 = new filter::sink<oT>(logger);
    pipeline.add_filter(std::unique_ptr<Filter>(sink_filter_0));

    auto sink_filter_1 = new filter::sink<oT>(logger);
    pipeline.add_filter(std::unique_ptr<Filter>(sink_filter_1));

    pipeline.link<oT>(source_filter, buffer_filter, fmt_source);
    pipeline.link<oT>(buffer_filter, iir_filter, fmt_buffer);
    pipeline.link<oT>(iir_filter, roll_filter, fmt_buffer);
    pipeline.link<oT>(roll_filter, fhr_filter, fmt_roll);
    pipeline.link<oT>(fhr_filter, sink_filter_0, fmt_fhr);
    pipeline.link<oT>(fhr_filter, sink_filter_1, fmt_fhr);

    data_handler.reinit_queue(common::data::type::us, elt_size, 1000);

    std::thread pipeline_th(pipeline_th_func, std::ref(pipeline));
    std::thread producer_th(producer_th_func, std::ref(producer));

    producer_th.join();
    pipeline.stop();
    pipeline_th.join();

    pipeline.print_stats();

    return 0;
}
