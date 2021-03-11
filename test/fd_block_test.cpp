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

    cnpy::NpyArray a_np       = cnpy::npz_load(filename_params, "a");
    cnpy::NpyArray b_np       = cnpy::npz_load(filename_params, "b");
    cnpy::NpyArray nskip_np   = cnpy::npz_load(filename_params, "nskip");
    cnpy::NpyArray nperseg_np = cnpy::npz_load(filename_params, "nperseg");
    cnpy::NpyArray nfft_np    = cnpy::npz_load(filename_params, "nfft");

    arma::vec b(b_np.data<double>(), b_np.shape.at(0));
    arma::vec a(a_np.data<double>(), a_np.shape.at(0));
    arma::uword nskip(*nskip_np.data<arma::uword>());
    arma::uword nperseg(*nperseg_np.data<arma::uword>());
    arma::uword nfft(*nfft_np.data<arma::uword>());

    common::Logger logger(spdlog::stdout_color_mt("dsp"));
    logger->set_level(spdlog::level::info);

    Pipeline pipeline(logger);

    auto source_filter = std::make_unique<NpySource<T>>(logger, filename_in);
    auto source_h = pipeline.add_filter(std::move(source_filter));
    auto fmt_data = source_filter->get_fmt();

    auto iir_filter = std::make_unique<filter::IIR<T, double>>(logger, b, a);
    auto iir_h = pipeline.add_filter(std::move(iir_filter));

    auto roll_filter = std::make_unique<filter::Roll<T>>(logger, nperseg, nskip);
    auto roll_h = pipeline.add_filter(std::move(roll_filter));

    auto fd_filter = std::make_unique<filter::FD<T, double>>(logger, nfft, arma::vec(nfft, arma::fill::ones));
    auto fd_h = pipeline.add_filter(std::move(fd_filter));

    auto sink_filter = std::make_unique<NpySink<T>>(logger, fmt_data);
    auto sink_h = pipeline.add_filter(std::move(sink_filter));

    pipeline.link<T>(source_h, "out", iir_h , "in");
    pipeline.link<T>(iir_h   , "out", roll_h, "in");
    pipeline.link<T>(roll_h  , "out", fd_h  , "in");
    pipeline.link<double>(fd_h, "out", sink_h, "in");

    Format fmt_in { nskip, fmt_data.n_cols, fmt_data.n_slices };
    Format fmt_fd { nfft, fmt_in.n_cols, fmt_in.n_slices };
    Format fmt_out{ 1, fmt_in.n_cols, fmt_in.n_slices };
    source_h->set_output_format(fmt_in, "out");
    iir_h->set_input_format(fmt_in, "in");
    iir_h->set_output_format(fmt_in, "out");
    roll_h->set_input_format(fmt_in, "in");
    roll_h->set_output_format(fmt_fd, "out");
    fd_h->set_input_format(fmt_fd, "in");
    fd_h->set_output_format(fmt_out, "out");
    sink_h->set_input_format(fmt_out, "in");
    if (pipeline.negotiate_format() != Contract::supported_format)
        throw dsp_error(Errc::format_negotiation_failed);

    std::cout << "Input:\n"
              << "  type: " << typeid(T).name() << "\n"
              << "  chunk size: (" << fmt_in.n_rows << "," << fmt_in.n_cols << "," << fmt_in.n_slices << ")\n"
              << "  nb frames total: " << fmt_data.n_rows << "\n"
              << "------------------------------\n"
              << "Filter params:\n"
              << "   nfft:  " << nfft << "\n"
              << "   nskip: " << nskip << "\n"
              << "   b: "; b.t().print();
    std::cout << "   a: "; a.t().print();
    std::cout << "------------------------------\n";

    sink_filter->dump(filename_out);

    pipeline.print_stats();

    return 0;
}


