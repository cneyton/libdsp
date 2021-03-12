#include "test_utils.h"

#include "dsp/buffer_filter.h"

#include "spdlog/sinks/stdout_color_sinks.h"

using T = double;

int main(int argc, char * argv[])
{
    if (argc != 4)
        return -1;

    std::string filename_in(argv[1]);
    std::string filename_out(argv[2]);
    std::string filename_params(argv[3]);

    cnpy::NpyArray n_buffer_np = cnpy::npz_load(filename_params, "fdskip");

    arma::uword n_buffer(*n_buffer_np.data<arma::uword>());

    common::Logger logger(spdlog::stdout_color_mt("dsp"));
    logger->set_level(spdlog::level::info);

    Pipeline pipeline(logger);

    auto source_filter = std::make_unique<NpySource<T>>(logger, filename_in);
    auto fmt_data = source_filter->get_fmt();
    auto source_h = pipeline.add_filter(std::move(source_filter));

    auto buffer_filter = std::make_unique<filter::Buffer<T>>(logger);
    auto buffer_h = pipeline.add_filter(std::move(buffer_filter));

    auto sink_filter = std::make_unique<NpySink<T>>(logger, fmt_data);
    auto sink_p = sink_filter.get();
    auto sink_h = pipeline.add_filter(std::move(sink_filter));

    pipeline.link<T>(source_h, "out", buffer_h, "in");
    pipeline.link<T>(buffer_h, "out", sink_h, "in");

    Format fmt_in  { 1, fmt_data.n_cols, fmt_data.n_slices };
    Format fmt_out { n_buffer, fmt_in.n_cols, fmt_in.n_slices };
    source_h->set_output_format(fmt_in, "out");
    buffer_h->set_input_format(fmt_in, "in");
    buffer_h->set_output_format(fmt_out, "out");
    sink_h->set_input_format(fmt_out, "in");
    if (pipeline.negotiate_format() != Contract::supported_format)
        throw dsp_error(Errc::format_negotiation_failed);

    std::cout << "Input:\n"
              << "  type: " << typeid(T).name() << "\n"
              << "  chunk size: (" << fmt_in.n_rows << "," << fmt_in.n_cols << "," << fmt_in.n_slices << ")\n"
              << "  nb frames total: " << fmt_data.n_rows << "\n"
              << "------------------------------\n"
              << "Filter params:\n"
              << "  n_buffer:     " << n_buffer << "\n"
              << "------------------------------\n";

    sink_p->dump(filename_out);

    pipeline.print_stats();

    return 0;
}
