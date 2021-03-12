#include "test_utils.h"

#include "dsp/roll_filter.h"

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


    cnpy::NpyArray nskip_np   = cnpy::npz_load(filename_params, "nskip");
    cnpy::NpyArray nperseg_np = cnpy::npz_load(filename_params, "nperseg");
    cnpy::NpyArray n_in_np    = cnpy::npz_load(filename_params, "n_in");
    cnpy::NpyArray n_out_np   = cnpy::npz_load(filename_params, "n_out");

    arma::uword nskip(*nskip_np.data<arma::uword>());
    arma::uword nperseg(*nperseg_np.data<arma::uword>());
    arma::uword n_in(*n_in_np.data<arma::uword>());
    arma::uword n_out(*n_out_np.data<arma::uword>());


    auto source_filter = std::make_unique<NpySource<T>>(logger, filename_in);
    auto fmt_data = source_filter->get_fmt();
    auto source_h = pipeline.add_filter(std::move(source_filter));

    auto roll_filter = std::make_unique<filter::Roll<T>>(logger, nperseg, nskip);
    auto roll_h = pipeline.add_filter(std::move(roll_filter));

    auto sink_filter = std::make_unique<NpySink<T>>(logger, fmt_data);
    auto sink_p = sink_filter.get();
    auto sink_h = pipeline.add_filter(std::move(sink_filter));

    pipeline.link<T>(source_h, "out", roll_h, "in");
    pipeline.link<T>(roll_h, "out", sink_h, "in");

    Format fmt_in { n_in, fmt_data.n_cols, fmt_data.n_slices };
    Format fmt_out { n_out, fmt_in.n_cols, fmt_in.n_slices };
    source_h->set_output_format(fmt_in, "out");
    roll_h->set_input_format(fmt_in, "in");
    roll_h->set_output_format(fmt_out, "out");
    sink_h->set_input_format(fmt_out, "in");
    if (pipeline.negotiate_format() != Contract::supported_format)
        throw dsp_error(Errc::format_negotiation_failed);

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

    sink_p->dump(filename_out);

    pipeline.print_stats();

    return 0;
}
