#include "test_utils.h"

#include "iir_filter.h"
#include "roll_filter.h"
#include "buffer_filter.h"
#include "fd_filter.h"
#include "fhr_filter.h"

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

    cnpy::NpyArray period_np     = cnpy::npz_load(filename_params, "period");
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

    arma::uword period(*period_np.data<arma::uword>());

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
    Pipeline pipeline(logger);
    Handler data_handler(logger, &pipeline);
    Producer<T_iq> producer(logger, &data_handler, filename_in, period);

    auto fmt_data = producer.get_fmt();
    std::cout << "Input:\n"
              << "  size: (" << fmt_data.n_rows << "," << fmt_data.n_cols << "," << fmt_data.n_slices << ")\n"
              << "  period: " << period << "\n"
              << "------------------------------\n"
              << "Filters params:\n"
              << "  nfft:       " << nfft << "\n"
              << "  nskip:      " << nskip << "\n"
              << "  fdskip:     " << fdskip << "\n"
              << "  fdperseg:   " << fdperseg << "\n"
              << "  radius:     " << radius << "\n"
              << "  period_max: " << period_max << "\n"
              << "  threshold:  " << threshold << "\n"
              << "  b1:\n"; b1.t().print();
    std::cout << "  a1:\n"; a1.t().print();
    std::cout << "  b2:\n"; b2.t().print();
    std::cout << "  a2:\n"; a2.t().print();
    std::cout << "------------------------------\n";

    arma::SizeCube fmt_in(nskip, fmt_data.n_cols, fmt_data.n_slices);
    auto source_filter = new filter::source<T_iq, T_iq>(logger, &data_handler, common::data::type::us);
    source_filter->set_chunk_size(fmt_in.n_rows, fmt_in.n_cols, fmt_in.n_slices);
    pipeline.add_filter(std::unique_ptr<Filter>(source_filter));

    auto iir_filter_iq = new filter::iir<T_iq, double>(logger, fmt_in.n_cols * fmt_in.n_slices, b1, a1);
    pipeline.add_filter(std::unique_ptr<Filter>(iir_filter_iq));

    arma::SizeCube fmt_roll_iq(nfft, fmt_in.n_cols, fmt_in.n_slices);
    auto roll_filter_iq = new filter::roll<T_iq>(logger, 1);
    pipeline.add_filter(std::unique_ptr<Filter>(roll_filter_iq));

    arma::SizeCube fmt_fd(1, fmt_in.n_cols, fmt_in.n_slices);
    auto fd_filter = new filter::fd<T_iq, T_fd>(logger, nfft, arma::vec(nfft, arma::fill::ones));
    pipeline.add_filter(std::unique_ptr<Filter>(fd_filter));

    arma::SizeCube fmt_buffer(fdskip, fmt_in.n_cols, fmt_in.n_slices);
    auto buffer_filter = new filter::buffer<T_fd>(logger);
    pipeline.add_filter(std::unique_ptr<Filter>(buffer_filter));

    auto iir_filter_fd = new filter::iir<T_fd, double>(logger, fmt_in.n_cols * fmt_in.n_slices, b2, a2);
    pipeline.add_filter(std::unique_ptr<Filter>(iir_filter_fd));

    arma::SizeCube fmt_roll_fd(fdperseg, fmt_in.n_cols, fmt_in.n_slices);
    auto roll_filter = new filter::roll<T_fd>(logger, 1);
    pipeline.add_filter(std::unique_ptr<Filter>(roll_filter));

    auto fhr_filter = new filter::fhr<T_fd, T_fd, T_fd>(logger, radius, period_max, threshold);
    pipeline.add_filter(std::unique_ptr<Filter>(fhr_filter));

    arma::SizeCube fmt_out(1, fmt_in.n_cols, fmt_in.n_slices);
    auto sink_filter_0 = new NpySink<T_fd>(logger, fmt_data);
    pipeline.add_filter(std::unique_ptr<Filter>(sink_filter_0));

    auto sink_filter_1 = new NpySink<T_fd>(logger, fmt_data);
    pipeline.add_filter(std::unique_ptr<Filter>(sink_filter_1));

    pipeline.link<T_iq>(source_filter, iir_filter_iq, fmt_in);
    pipeline.link<T_iq>(iir_filter_iq, roll_filter_iq, fmt_in);
    pipeline.link<T_iq>(roll_filter_iq, fd_filter, fmt_roll_iq);
    pipeline.link<T_fd>(fd_filter, buffer_filter, fmt_fd);
    pipeline.link<T_fd>(buffer_filter, iir_filter_fd, fmt_buffer);
    pipeline.link<T_fd>(iir_filter_fd, roll_filter, fmt_buffer);
    pipeline.link<T_fd>(roll_filter, fhr_filter, fmt_roll_fd);
    pipeline.link<T_fd>(fhr_filter, sink_filter_0, fmt_out);
    pipeline.link<T_fd>(fhr_filter, sink_filter_1, fmt_out);

    std::thread pipeline_th(&Pipeline::run, &pipeline);
    std::thread producer_th(&Producer<T_iq>::run, &producer);

    producer_th.join();
    pipeline_th.join();

    sink_filter_0->dump("fhr_" + filename_out);
    sink_filter_1->dump("corr_" + filename_out);

    pipeline.print_stats();

    return 0;
}
