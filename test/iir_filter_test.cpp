#include "test_utils.h"

#include "dsp/iir_filter.h"

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

    //cnpy::NpyArray period_np  = cnpy::npz_load(filename_params, "period");
    cnpy::NpyArray nskip_np   = cnpy::npz_load(filename_params, "nskip");
    cnpy::NpyArray a_np       = cnpy::npz_load(filename_params, "a");
    cnpy::NpyArray b_np       = cnpy::npz_load(filename_params, "b");

    //arma::uword period(*period_np.data<arma::uword>());
    arma::uword nskip(*nskip_np.data<arma::uword>());
    arma::vec b(b_np.data<double>(), b_np.shape.at(0));
    arma::vec a(a_np.data<double>(), a_np.shape.at(0));

    common::Logger logger(spdlog::stdout_color_mt("dsp"));
    logger->set_level(spdlog::level::info);

    Pipeline pipeline(logger);

    auto source_filter = std::make_unique<NpySource<T>>(logger, filename_in);
    auto fmt_data = source_filter->get_fmt();
    auto source_h = pipeline.add_filter(std::move(source_filter));

    auto iir_filter = std::make_unique<filter::IIR<T, double>>(logger, b, a);
    auto iir_h = pipeline.add_filter(std::move(iir_filter));

    auto sink_filter = std::make_unique<NpySink<T>>(logger, fmt_data);
    auto sink_p = sink_filter.get();
    auto sink_h = pipeline.add_filter(std::move(sink_filter));

    pipeline.link<T>(source_h, "out", iir_h, "in");
    pipeline.link<T>(iir_h, "out", sink_h, "in");

    Format fmt { nskip, fmt_data.n_cols, fmt_data.n_slices };
    source_h->set_output_format(fmt, "out");
    iir_h->set_input_format(fmt, "in");
    iir_h->set_output_format(fmt, "out");
    sink_h->set_input_format(fmt, "in");
    if (pipeline.negotiate_format() != Contract::supported_format)
        throw dsp_error(Errc::format_negotiation_failed);

    std::cout << "Input:\n"
              << "  type: " << typeid(T).name() << "\n"
              << "  chunk size: (" << fmt.n_rows << "," << fmt.n_cols << "," << fmt.n_slices << ")\n"
              << "  nb frames total: " << fmt_data.n_rows << "\n"
              << "------------------------------\n"
              << "Filter params:\n"
              << "  b: "; b.t().print();
    std::cout << "  a: "; a.t().print();
    std::cout << "------------------------------\n";

    sink_p->dump(filename_out);

    pipeline.print_stats();

    return 0;
}
