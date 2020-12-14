#include "test_utils.h"

#include "dsp/iir_filter.h"
#include "dsp/roll_filter.h"
#include "dsp/fd_filter.h"

#include "spdlog/common.h"
#include "spdlog/sinks/stdout_color_sinks.h"

using T = arma::cx_double;

int main(int argc, char * argv[])
{
    if (argc != 4)
        return -1;

    std::string filename_in(argv[1]);
    std::string filename_out(argv[2]);
    std::string filename_params(argv[3]);

    common::Logger logger(spdlog::stdout_color_mt("dsp"));
    logger->set_level(spdlog::level::info);

    Pipeline pipeline(logger);


    cnpy::NpyArray nskip_np  = cnpy::npz_load(filename_params, "nskip");
    cnpy::NpyArray n_in_np   = cnpy::npz_load(filename_params, "n_in");
    cnpy::NpyArray n_out_np  = cnpy::npz_load(filename_params, "n_out");

    arma::uword nskip(*nskip_np.data<arma::uword>());
    arma::uword n_in(*n_in_np.data<arma::uword>());
    arma::uword n_out(*n_out_np.data<arma::uword>());


    auto source_filter = new NpySource<T>(logger, filename_in);
    pipeline.add_filter(std::unique_ptr<Filter>(source_filter));
    auto fmt_data = source_filter->get_fmt();
    arma::SizeCube fmt_in(n_in, fmt_data.n_cols, fmt_data.n_slices);

    arma::SizeCube fmt_out(n_out, fmt_in.n_cols, fmt_in.n_slices);
    auto roll_filter = new filter::Roll<T>(logger, nskip);
    pipeline.add_filter(std::unique_ptr<Filter>(roll_filter));

    auto sink_filter = new NpySink<T>(logger, fmt_data);
    pipeline.add_filter(std::unique_ptr<Filter>(sink_filter));

    pipeline.link<T>(source_filter, roll_filter, fmt_in);
    pipeline.link<T>(roll_filter, sink_filter, fmt_out);

    std::cout << "Input:\n"
              << "  type: " << typeid(T).name() << "\n"
              << "  size: (" << fmt_in.n_rows << "," << fmt_in.n_cols << "," << fmt_in.n_slices << ")\n"
              << "  nb frames total: " << fmt_data.n_rows << "\n"
              << "------------------------------\n"
              << "Filter params:\n"
              << "   n_in:  " << n_in << "\n"
              << "   n_out: " << n_in << "\n"
              << "   nskip: " << nskip << "\n"
              << "------------------------------\n";

    sink_filter->dump(filename_out);

    pipeline.print_stats();

    return 0;
}
