#include "test_utils.h"

#include "iir_filter.h"
#include "roll_filter.h"
#include "fd_filter.h"

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

    cnpy::NpyArray a_np     = cnpy::npz_load(filename_params, "a");
    cnpy::NpyArray b_np     = cnpy::npz_load(filename_params, "b");
    cnpy::NpyArray nskip_np = cnpy::npz_load(filename_params, "nskip");
    cnpy::NpyArray nfft_np  = cnpy::npz_load(filename_params, "nfft");

    arma::vec b(b_np.data<double>(), b_np.shape.at(0));
    arma::vec a(a_np.data<double>(), a_np.shape.at(0));
    arma::uword nskip(*nskip_np.data<arma::uword>());
    arma::uword nfft(*nfft_np.data<arma::uword>());

    common::Logger logger(spdlog::stdout_color_mt("dsp"));
    logger->set_level(spdlog::level::info);

    Pipeline pipeline(logger);

    auto source_filter = new NpySource<T>(logger, filename_in);
    pipeline.add_filter(std::unique_ptr<Filter>(source_filter));
    auto fmt_data = source_filter->get_fmt();
    arma::SizeCube fmt_in(nskip, fmt_data.n_cols, fmt_data.n_slices);

    auto iir_filter = new filter::iir<T, double>(logger, fmt_in.n_cols * fmt_in.n_slices, b, a);
    pipeline.add_filter(std::unique_ptr<Filter>(iir_filter));

    arma::SizeCube fmt_fd(nfft, fmt_in.n_cols, fmt_in.n_slices);
    auto roll_filter = new filter::roll<T>(logger, 1);
    pipeline.add_filter(std::unique_ptr<Filter>(roll_filter));

    arma::SizeCube fmt_out(1, fmt_in.n_cols, fmt_in.n_slices);
    auto fd_filter = new filter::fd<T, double>(logger, nfft, arma::vec(nfft, arma::fill::ones));
    pipeline.add_filter(std::unique_ptr<Filter>(fd_filter));

    auto sink_filter = new NpySink<double>(logger, fmt_data);
    pipeline.add_filter(std::unique_ptr<Filter>(sink_filter));

    pipeline.link<T>(source_filter, iir_filter, fmt_in);
    pipeline.link<T>(iir_filter, roll_filter, fmt_in);
    pipeline.link<T>(roll_filter, fd_filter, fmt_fd);
    pipeline.link<double>(fd_filter, sink_filter, fmt_out);

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


