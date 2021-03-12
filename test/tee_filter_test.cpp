#include "test_utils.h"

#include "dsp/tee_filter.h"

#include "spdlog/common.h"
#include "spdlog/sinks/stdout_color_sinks.h"

using T = double;

constexpr uint16_t nb_frames  = 1;
constexpr uint     N = 2;

int main(int argc, char * argv[])
{
    if (argc != 4)
        return -1;

    std::string filename_in(argv[1]);
    std::string filename_out(argv[2]);

    common::Logger logger(spdlog::stdout_color_mt("dsp"));
    logger->set_level(spdlog::level::info);

    Pipeline pipeline(logger);

    auto source_filter = std::make_unique<NpySource<T>>(logger, filename_in);
    auto fmt_data = source_filter->get_fmt();
    auto source_h = pipeline.add_filter(std::move(source_filter));

    auto tee_filter = std::make_unique<filter::Tee<T, N>>(logger);
    auto tee_h = pipeline.add_filter(std::move(tee_filter));

    auto sink_filter_0 = std::make_unique<NpySink<T>>(logger, fmt_data);
    auto sink_p1 = sink_filter_0.get();
    auto sink0_h = pipeline.add_filter(std::move(sink_filter_0));

    auto sink_filter_1 = std::make_unique<NpySink<T>>(logger, fmt_data);
    auto sink_p2 = sink_filter_1.get();
    auto sink1_h = pipeline.add_filter(std::move(sink_filter_1));

    pipeline.link<T>(source_h, "out", tee_h  , "in");
    pipeline.link<T>(tee_h   , "0"  , sink0_h, "in");
    pipeline.link<T>(tee_h   , "1"  , sink1_h, "in");

    Format fmt { nb_frames, fmt_data.n_cols, fmt_data.n_slices };
    source_h->set_output_format(fmt, "out");
    tee_h->set_input_format(fmt, "in");
    tee_h->set_output_format(fmt, "0");
    tee_h->set_output_format(fmt, "1");
    sink0_h->set_input_format(fmt, "in");
    sink1_h->set_input_format(fmt, "in");
    if (pipeline.negotiate_format() != Contract::supported_format)
        throw dsp_error(Errc::format_negotiation_failed);

    std::cout << "Input:\n"
              << "  type: " << typeid(T).name() << "\n"
              << "  chunk size: (" << fmt.n_rows << "," << fmt.n_cols << "," << fmt.n_slices << ")\n"
              << "  nb frames total: " << fmt_data.n_rows << "\n"
              << "------------------------------\n";

    sink_p1->dump("out_1_" + filename_out);
    sink_p2->dump("out_2_" + filename_out);

    pipeline.print_stats();

    return 0;
}

