#include "test_utils.h"

#include "iir_filter.h"
#include "roll_filter.h"
#include "buffer_filter.h"
#include "fhr_filter.h"

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

    auto source_filter = new NpySource<T>(logger, filename_in);
    pipeline.add_filter(std::unique_ptr<Filter>(source_filter));
    auto fmt_data = source_filter->get_fmt();
    arma::SizeCube fmt_in(1, fmt_data.n_cols, fmt_data.n_slices);

    arma::SizeCube fmt_out(n_buffer, fmt_in.n_cols, fmt_in.n_slices);
    auto buffer_filter = new filter::buffer<T>(logger);
    pipeline.add_filter(std::unique_ptr<Filter>(buffer_filter));

    auto sink_filter = new NpySink<T>(logger, fmt_data);
    pipeline.add_filter(std::unique_ptr<Filter>(sink_filter));

    pipeline.link<T>(source_filter, buffer_filter, fmt_in);
    pipeline.link<T>(buffer_filter, sink_filter, fmt_out);

    std::cout << "Input:\n"
              << "  type: " << typeid(T).name() << "\n"
              << "  chunk size: (" << fmt_in.n_rows << "," << fmt_in.n_cols << "," << fmt_in.n_slices << ")\n"
              << "  nb frames total: " << fmt_data.n_rows << "\n"
              << "------------------------------\n"
              << "Filter params:\n"
              << "  n_buffer:     " << n_buffer << "\n"
              << "------------------------------\n";

    sink_filter->dump(filename_out);

    pipeline.print_stats();

    return 0;
}
