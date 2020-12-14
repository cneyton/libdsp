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

    auto source_filter = new NpySource<T>(logger, filename_in);
    pipeline.add_filter(std::unique_ptr<Filter>(source_filter));
    auto fmt_data = source_filter->get_fmt();
    arma::SizeCube fmt(nb_frames, fmt_data.n_cols, fmt_data.n_slices);

    auto sink_filter_0 = new NpySink<T>(logger, fmt_data);
    pipeline.add_filter(std::unique_ptr<Filter>(sink_filter_0));

    auto sink_filter_1 = new NpySink<T>(logger, fmt_data);
    pipeline.add_filter(std::unique_ptr<Filter>(sink_filter_1));

    auto tee_filter = new filter::Tee<T, N>(logger);
    pipeline.add_filter(std::unique_ptr<Filter>(tee_filter));

    pipeline.link<T>(source_filter, tee_filter, fmt);
    pipeline.link<T>(tee_filter   , sink_filter_0, fmt);
    pipeline.link<T>(tee_filter   , sink_filter_1, fmt);

    std::cout << "Input:\n"
              << "  type: " << typeid(T).name() << "\n"
              << "  chunk size: (" << fmt.n_rows << "," << fmt.n_cols << "," << fmt.n_slices << ")\n"
              << "  nb frames total: " << fmt_data.n_rows << "\n"
              << "------------------------------\n";

    sink_filter_0->dump("out_1_" + filename_out);
    sink_filter_1->dump("out_2_" + filename_out);

    pipeline.print_stats();

    return 0;
}

