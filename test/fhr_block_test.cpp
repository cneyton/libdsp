#include "test_utils.h"

#include "dsp/iir_filter.h"
#include "dsp/roll_filter.h"
#include "dsp/buffer_filter.h"
#include "dsp/fhr_filter.h"

#include "spdlog/sinks/stdout_color_sinks.h"

using T = double;

int main(int argc, char * argv[])
{
    if (argc != 4)
        return -1;

    std::string filename_in(argv[1]);
    std::string filename_out(argv[2]);
    std::string filename_params(argv[3]);

    cnpy::NpyArray a_np          = cnpy::npz_load(filename_params, "a");
    cnpy::NpyArray b_np          = cnpy::npz_load(filename_params, "b");
    cnpy::NpyArray fdskip_np     = cnpy::npz_load(filename_params, "fdskip");
    cnpy::NpyArray fdperseg_np   = cnpy::npz_load(filename_params, "fdperseg");
    cnpy::NpyArray radius_np     = cnpy::npz_load(filename_params, "radius");
    cnpy::NpyArray period_max_np = cnpy::npz_load(filename_params, "period_max");
    cnpy::NpyArray threshold_np  = cnpy::npz_load(filename_params, "threshold");

    arma::vec   b(b_np.data<double>(), b_np.shape.at(0));
    arma::vec   a(a_np.data<double>(), a_np.shape.at(0));
    arma::uword fdskip(*fdskip_np.data<arma::uword>());
    arma::uword fdperseg(*fdperseg_np.data<arma::uword>());
    arma::uword radius(*radius_np.data<arma::uword>());
    arma::uword period_max(*period_max_np.data<arma::uword>());
    T           threshold(*threshold_np.data<T>());


    common::Logger logger(spdlog::stdout_color_mt("dsp"));
    logger->set_level(spdlog::level::info);

    Pipeline pipeline(logger);

    auto source_filter = std::make_unique<NpySource<T>>(logger, filename_in);
    auto fmt_data = source_filter->get_fmt();
    auto source_h = pipeline.add_filter(std::move(source_filter));

    auto buffer_filter = std::make_unique<filter::Buffer<T>>(logger);
    auto buffer_h = pipeline.add_filter(std::move(buffer_filter));

    auto iir_filter = std::make_unique<filter::IIR<T, double>>(logger, b, a);
    auto iir_h = pipeline.add_filter(std::move(iir_filter));

    arma::uword nperseg = fdperseg / fdskip;
    auto roll_filter = std::make_unique<filter::Roll<T>>(logger, nperseg, 1);
    auto roll_h = pipeline.add_filter(std::move(roll_filter));

    auto fhr_filter = std::make_unique<filter::FHR<T, T, T>>(logger, radius, period_max, threshold);
    auto fhr_h = pipeline.add_filter(std::move(fhr_filter));

    auto sink_filter_0 = std::make_unique<NpySink<T>>(logger, fmt_data);
    auto sink_p0 = sink_filter_0.get();
    auto sink0_h = pipeline.add_filter(std::move(sink_filter_0));

    auto sink_filter_1 = std::make_unique<NpySink<T>>(logger, fmt_data);
    auto sink_p1 = sink_filter_1.get();
    auto sink1_h = pipeline.add_filter(std::move(sink_filter_1));

    pipeline.link<T>(source_h, "out", buffer_h, "in");
    pipeline.link<T>(buffer_h, "out", iir_h   , "in");
    pipeline.link<T>(iir_h   , "out", roll_h  , "in");
    pipeline.link<T>(roll_h  , "out", fhr_h   , "in");
    pipeline.link<T>(fhr_h   , "fhr", sink0_h , "in");
    pipeline.link<T>(fhr_h   , "cor", sink1_h , "in");

    Format fmt_in     { 1       , fmt_data.n_cols, fmt_data.n_slices };
    Format fmt_buffer { fdskip  , fmt_in.n_cols, fmt_in.n_slices };
    Format fmt_roll   { fdperseg, fmt_in.n_cols, fmt_in.n_slices };
    Format fmt_out    { 1       , fmt_in.n_cols, fmt_in.n_slices };
    source_h->set_output_format(fmt_in, "out");
    buffer_h->set_input_format(fmt_in, "in");
    buffer_h->set_output_format(fmt_buffer, "out");
    iir_h->set_input_format(fmt_buffer, "in");
    iir_h->set_output_format(fmt_buffer, "out");
    roll_h->set_input_format(fmt_buffer, "in");
    roll_h->set_output_format(fmt_roll, "out");
    fhr_h->set_input_format(fmt_roll, "in");
    fhr_h->set_output_format(fmt_out, "fhr");
    fhr_h->set_output_format(fmt_out, "cor");
    sink0_h->set_input_format(fmt_out, "in");
    sink1_h->set_input_format(fmt_out, "in");
    if (pipeline.negotiate_format() != Contract::supported_format)
        throw dsp_error(Errc::format_negotiation_failed);

    std::cout << "Input:\n"
              << "  type: " << typeid(T).name() << "\n"
              << "  chunk size: (" << fmt_in.n_rows << "," << fmt_in.n_cols << "," << fmt_in.n_slices << ")\n"
              << "  nb frames total: " << fmt_data.n_rows << "\n"
              << "------------------------------\n"
              << "Filter params:\n"
              << "  fdskip:     " << fdskip << "\n"
              << "  fdperseg:   " << fdperseg << "\n"
              << "  radius:     " << radius << "\n"
              << "  period_max: " << period_max << "\n"
              << "  threshold:  " << threshold << "\n"
              << "  b: "; b.t().print();
    std::cout << "  a: "; a.t().print();
    std::cout << "------------------------------\n";

    sink_p0->dump("fhr_" + filename_out);
    sink_p1->dump("corr_" + filename_out);

    pipeline.print_stats();

    return 0;
}
