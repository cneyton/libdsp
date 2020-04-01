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

    auto source_filter = new NpySource<T>(logger, filename_in);
    pipeline.add_filter(std::unique_ptr<Filter>(source_filter));
    auto fmt_data = source_filter->get_fmt();
    arma::SizeCube fmt_in(1, fmt_data.n_cols, fmt_data.n_slices);

    arma::SizeCube fmt_buffer(fdskip, fmt_in.n_cols, fmt_in.n_slices);
    auto buffer_filter = new filter::buffer<T>(logger);
    pipeline.add_filter(std::unique_ptr<Filter>(buffer_filter));

    auto iir_filter = new filter::iir<T, double>(logger, fmt_in.n_cols * fmt_in.n_slices, b, a);
    pipeline.add_filter(std::unique_ptr<Filter>(iir_filter));

    arma::SizeCube fmt_roll(fdperseg, fmt_in.n_cols, fmt_in.n_slices);
    auto roll_filter = new filter::roll<T>(logger, 1);
    pipeline.add_filter(std::unique_ptr<Filter>(roll_filter));

    arma::SizeCube fmt_out(1, fmt_in.n_cols, fmt_in.n_slices);
    auto fhr_filter = new filter::fhr<T, T, T>(logger, radius, period_max, threshold);
    pipeline.add_filter(std::unique_ptr<Filter>(fhr_filter));

    auto sink_filter_0 = new NpySink<T>(logger, fmt_data);
    pipeline.add_filter(std::unique_ptr<Filter>(sink_filter_0));

    auto sink_filter_1 = new NpySink<T>(logger, fmt_data);
    pipeline.add_filter(std::unique_ptr<Filter>(sink_filter_1));

    pipeline.link<T>(source_filter, buffer_filter, fmt_in);
    pipeline.link<T>(buffer_filter, iir_filter, fmt_buffer);
    pipeline.link<T>(iir_filter, roll_filter, fmt_buffer);
    pipeline.link<T>(roll_filter, fhr_filter, fmt_roll);
    pipeline.link<T>(fhr_filter, sink_filter_0, fmt_out);
    pipeline.link<T>(fhr_filter, sink_filter_1, fmt_out);

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

    sink_filter_0->dump("fhr_" + filename_out);
    sink_filter_1->dump("corr_" + filename_out);

    pipeline.print_stats();

    return 0;
}
