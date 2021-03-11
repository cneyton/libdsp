#include "test_utils.h"

#include "dsp/iir_filter.h"
#include "dsp/roll_filter.h"
#include "dsp/buffer_filter.h"
#include "dsp/fd_filter.h"
#include "dsp/fhr_filter.h"

#include "spdlog/sinks/stdout_color_sinks.h"

using T_iq = arma::cx_double;
using T_fd = double;

int main(int argc, char * argv[])
{
    if (argc != 4)
        return -1;

    std::string filename_in(argv[1]);
    std::string filename_out(argv[2]);
    std::string filename_params(argv[3]);

    cnpy::NpyArray a1_np         = cnpy::npz_load(filename_params, "a1");
    cnpy::NpyArray b1_np         = cnpy::npz_load(filename_params, "b1");
    cnpy::NpyArray a2_np         = cnpy::npz_load(filename_params, "a2");
    cnpy::NpyArray b2_np         = cnpy::npz_load(filename_params, "b2");
    cnpy::NpyArray nskip_np      = cnpy::npz_load(filename_params, "nskip");
    cnpy::NpyArray nfft_np       = cnpy::npz_load(filename_params, "nfft");
    cnpy::NpyArray fdskip_np     = cnpy::npz_load(filename_params, "fdskip");
    cnpy::NpyArray fdperseg_np   = cnpy::npz_load(filename_params, "fdperseg");
    cnpy::NpyArray radius_np     = cnpy::npz_load(filename_params, "radius");
    cnpy::NpyArray period_max_np = cnpy::npz_load(filename_params, "period_max");
    cnpy::NpyArray threshold_np  = cnpy::npz_load(filename_params, "threshold");

    arma::vec   b1(b1_np.data<double>(), b1_np.shape.at(0));
    arma::vec   a1(a1_np.data<double>(), a1_np.shape.at(0));
    arma::vec   b2(b2_np.data<double>(), b2_np.shape.at(0));
    arma::vec   a2(a2_np.data<double>(), a2_np.shape.at(0));
    arma::uword nskip(*nskip_np.data<arma::uword>());
    arma::uword nfft(*nfft_np.data<arma::uword>());
    arma::uword fdskip(*fdskip_np.data<arma::uword>());
    arma::uword fdperseg(*fdperseg_np.data<arma::uword>());
    arma::uword radius(*radius_np.data<arma::uword>());
    arma::uword period_max(*period_max_np.data<arma::uword>());
    T_fd        threshold(*threshold_np.data<T_fd>());


    common::Logger logger(spdlog::stdout_color_mt("dsp"));
    logger->set_level(spdlog::level::info);

    Pipeline pipeline(logger);

    auto source_filter = std::make_unique<NpySource<T_iq>>(logger, filename_in);
    auto source_h = pipeline.add_filter(std::move(source_filter));
    auto fmt_data = source_filter->get_fmt();

    auto iir_filter_iq = std::make_unique<filter::IIR<T_iq, double>>(logger, b1, a1);
    auto iir_iq_h = pipeline.add_filter(std::move(iir_filter_iq));

    auto roll_filter_iq = std::make_unique<filter::Roll<T_iq>>(logger, 4, nskip);
    auto roll_iq_h = pipeline.add_filter(std::move(roll_filter_iq));

    auto fd_filter = std::make_unique<filter::FD<T_iq, T_fd>>(logger, nfft, arma::vec(nfft, arma::fill::ones));
    auto fd_h = pipeline.add_filter(std::move(fd_filter));

    auto buffer_filter = std::make_unique<filter::Buffer<T_fd>>(logger);
    auto buffer_h = pipeline.add_filter(std::move(buffer_filter));

    auto iir_filter_fd = std::make_unique<filter::IIR<T_fd, double>>(logger, b2, a2);
    auto iir_fd_h = pipeline.add_filter(std::move(iir_filter_fd));

    auto roll_fd_filter = std::make_unique<filter::Roll<T_fd>>(logger, fdperseg, fdskip);
    auto roll_fd_h = pipeline.add_filter(std::move(roll_fd_filter));

    auto fhr_filter = std::make_unique<filter::FHR<T_fd, T_fd, T_fd>>(logger, radius, period_max, threshold);
    auto fhr_h = pipeline.add_filter(std::move(fhr_filter));

    auto sink_filter_0 = std::make_unique<NpySink<T_fd>>(logger, fmt_data);
    auto sink0_h = pipeline.add_filter(std::move(sink_filter_0));

    auto sink_filter_1 = std::make_unique<NpySink<T_fd>>(logger, fmt_data);
    auto sink1_h = pipeline.add_filter(std::move(sink_filter_1));

    pipeline.link<T_iq>(source_h , "out", iir_iq_h , "in");
    pipeline.link<T_iq>(iir_iq_h , "out", roll_iq_h, "in");
    pipeline.link<T_iq>(roll_iq_h, "out", fd_h     , "in");
    pipeline.link<T_fd>(fd_h     , "out", buffer_h , "in");
    pipeline.link<T_fd>(buffer_h , "out", iir_fd_h , "in");
    pipeline.link<T_fd>(iir_fd_h , "out", roll_fd_h, "in");
    pipeline.link<T_fd>(roll_fd_h, "out", fhr_h    , "in");
    pipeline.link<T_fd>(fhr_h    , "fhr", sink0_h  , "in");
    pipeline.link<T_fd>(fhr_h    , "cor", sink1_h  , "in");

    Format fmt_in { nskip, fmt_data.n_cols, fmt_data.n_slices };
    Format fmt_roll_iq { nfft, fmt_in.n_cols, fmt_in.n_slices };
    Format fmt_fd{ 1, fmt_in.n_cols, fmt_in.n_slices };
    Format fmt_buffer { fdskip, fmt_in.n_cols, fmt_in.n_slices };
    Format fmt_roll_fd { fdperseg, fmt_in.n_cols, fmt_in.n_slices };
    Format fmt_out { 1, fmt_in.n_cols, fmt_in.n_slices };

    source_h->set_output_format(fmt_in, "out");
    iir_iq_h->set_input_format(fmt_in, "in");
    iir_iq_h->set_output_format(fmt_in, "out");
    roll_iq_h->set_input_format(fmt_in, "in");
    roll_iq_h->set_output_format(fmt_roll_iq, "out");
    fd_h->set_input_format(fmt_roll_iq, "in");
    fd_h->set_output_format(fmt_fd, "out");
    buffer_h->set_input_format(fmt_fd, "in");
    buffer_h->set_output_format(fmt_buffer, "out");
    iir_fd_h->set_input_format(fmt_buffer, "in");
    iir_fd_h->set_output_format(fmt_buffer, "out");
    roll_fd_h->set_input_format(fmt_buffer, "in");
    roll_fd_h->set_output_format(fmt_roll_fd, "out");
    fhr_h->set_input_format(fmt_roll_fd, "in");
    fhr_h->set_output_format(fmt_out, "fhr");
    fhr_h->set_output_format(fmt_out, "cor");
    sink0_h->set_input_format(fmt_out, "in");
    sink1_h->set_input_format(fmt_out, "in");
    if (pipeline.negotiate_format() != Contract::supported_format)
        throw dsp_error(Errc::format_negotiation_failed);

    std::cout << "Input:\n"
              << "  type: " << typeid(T_iq).name() << "\n"
              << "  chunk size: (" << fmt_in.n_rows << "," << fmt_in.n_cols << "," << fmt_in.n_slices << ")\n"
              << "  nb frames total: " << fmt_data.n_rows << "\n"
              << "------------------------------\n"
              << "Filters params:\n"
              << "  nfft:       " << nfft << "\n"
              << "  nskip:      " << nskip << "\n"
              << "  fdskip:     " << fdskip << "\n"
              << "  fdperseg:   " << fdperseg << "\n"
              << "  radius:     " << radius << "\n"
              << "  period_max: " << period_max << "\n"
              << "  threshold:  " << threshold << "\n"
              << "  b1: "; b1.t().print();
    std::cout << "  a1: "; a1.t().print();
    std::cout << "  b2: "; b2.t().print();
    std::cout << "  a2: "; a2.t().print();
    std::cout << "------------------------------\n";

    sink_filter_0->dump("fhr_" + filename_out);
    sink_filter_1->dump("corr_" + filename_out);

    pipeline.print_stats();

    return 0;
}
