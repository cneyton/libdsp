#include "test_utils.h"

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

    cnpy::NpyArray fdperseg_np   = cnpy::npz_load(filename_params, "fdperseg");
    cnpy::NpyArray radius_np     = cnpy::npz_load(filename_params, "radius");
    cnpy::NpyArray period_max_np = cnpy::npz_load(filename_params, "period_max");
    cnpy::NpyArray threshold_np  = cnpy::npz_load(filename_params, "threshold");

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

    auto fhr_filter = std::make_unique<filter::FHR<T, T, T>>(logger, radius, period_max, threshold);
    auto fhr_h = pipeline.add_filter(std::move(fhr_filter));

    auto sink_filter_0 = std::make_unique<NpySink<T>>(logger, fmt_data);
    auto sink_p0 = sink_filter_0.get();
    auto sink0_h = pipeline.add_filter(std::move(sink_filter_0));

    auto sink_filter_1 = std::make_unique<NpySink<T>>(logger, fmt_data);
    auto sink_p1 = sink_filter_1.get();
    auto sink1_h = pipeline.add_filter(std::move(sink_filter_1));

    pipeline.link<T>(source_h, "out", fhr_h  , "in");
    pipeline.link<T>(fhr_h   , "fhr", sink0_h, "in");
    pipeline.link<T>(fhr_h   , "cor", sink1_h, "in");

    Format fmt_in  { fdperseg, fmt_data.n_cols, fmt_data.n_slices };
    Format fmt_out {1, fmt_in.n_cols, fmt_in.n_slices };
    source_h->set_output_format(fmt_in, "out");
    fhr_h->set_input_format(fmt_in, "in");
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
              << "  fdperseg:   " << fdperseg << "\n"
              << "  radius:     " << radius << "\n"
              << "  period_max: " << period_max << "\n"
              << "  threshold:  " << threshold << "\n"
              << "------------------------------\n";

    sink_p0->dump("fhr_" + filename_out);
    sink_p1->dump("corr_" + filename_out);

    pipeline.print_stats();

    return 0;
}
