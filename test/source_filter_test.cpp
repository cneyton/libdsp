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

    auto source_filter = std::make_unique<NpySource<T>>(logger, filename_in, 1);
    auto fmt_data = source_filter->get_fmt();
    auto source_h = pipeline.add_filter(std::move(source_filter));

    auto sink_filter = std::make_unique<NpySink<T>>(logger, fmt_data);
    auto sink_p = sink_filter.get();
    auto sink_h = pipeline.add_filter(std::move(sink_filter));

    pipeline.link<T>(source_h, "out", sink_h, "in");

    Format fmt { 20, fmt_data.n_cols, fmt_data.n_slices };
    source_h->set_output_format(fmt, "out");
    sink_h->set_input_format(fmt, "in");
    if (pipeline.negotiate_format() != Contract::supported_format)
        throw dsp_error(Errc::format_negotiation_failed);

    std::cout << "Input:\n"
              << "  type: " << typeid(T).name() << "\n"
              << "  chunk size: (" << fmt.n_rows << "," << fmt.n_cols << "," << fmt.n_slices << ")\n"
              << "  nb frames total: " << fmt_data.n_rows << "\n"
              << "------------------------------\n";

    sink_p->dump(filename_out);

    pipeline.print_stats();

    return 0;
}
