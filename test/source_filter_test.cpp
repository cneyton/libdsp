#include "test_utils.h"

#include "spdlog/common.h"
#include "spdlog/sinks/stdout_color_sinks.h"

using T = arma::cx_double;

/* TODO: we should test different type of input/output <20-02-20, cneyton> */
int main(int argc, char * argv[])
{
    if (argc != 3)
        return -1;

    std::string filename_in(argv[1]);
    std::string filename_out(argv[2]);

    common::Logger logger(spdlog::stdout_color_mt("dsp"));
    logger->set_level(spdlog::level::debug);

    Pipeline pipeline(logger);

    auto source_filter = new NpySource<T>(logger, filename_in);
    pipeline.add_filter(std::unique_ptr<Filter>(source_filter));
    auto fmt_data = source_filter->get_fmt();

    auto sink_filter = new NpySink<T>(logger, fmt_data);
    pipeline.add_filter(std::unique_ptr<Filter>(sink_filter));

    arma::SizeCube fmt(20, fmt_data.n_cols, fmt_data.n_slices);
    pipeline.link<T>(*source_filter, *sink_filter, fmt);

    std::cout << "Input:\n"
              << "  type: " << typeid(T).name() << "\n"
              << "  chunk size: (" << fmt.n_rows << "," << fmt.n_cols << "," << fmt.n_slices << ")\n"
              << "  nb frames total: " << fmt_data.n_rows << "\n"
              << "------------------------------\n";

    sink_filter->dump(filename_out);

    pipeline.print_stats();

    return 0;
}
