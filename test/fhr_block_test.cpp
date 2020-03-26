#include "test_utils.h"

#include "iir_filter.h"
#include "roll_filter.h"
#include "buffer_filter.h"
#include "fhr_filter.h"

#include "spdlog/sinks/stdout_color_sinks.h"

using iT = double;
using oT = double;

int main(int argc, char * argv[])
{
    if (argc != 4)
        return -1;

    std::string filename_in(argv[1]);
    std::string filename_out(argv[2]);
    std::string filename_params(argv[3]);

    cnpy::NpyArray period_np     = cnpy::npz_load(filename_params, "period");
    cnpy::NpyArray a_np          = cnpy::npz_load(filename_params, "a");
    cnpy::NpyArray b_np          = cnpy::npz_load(filename_params, "b");
    cnpy::NpyArray fdskip_np     = cnpy::npz_load(filename_params, "fdskip");
    cnpy::NpyArray fdperseg_np   = cnpy::npz_load(filename_params, "fdperseg");
    cnpy::NpyArray radius_np     = cnpy::npz_load(filename_params, "radius");
    cnpy::NpyArray period_max_np = cnpy::npz_load(filename_params, "period_max");
    cnpy::NpyArray threshold_np  = cnpy::npz_load(filename_params, "threshold");

    arma::uword period(*period_np.data<arma::uword>());

    arma::vec   b(b_np.data<double>(), b_np.shape.at(0));
    arma::vec   a(a_np.data<double>(), a_np.shape.at(0));
    arma::uword fdskip(*fdskip_np.data<arma::uword>());
    arma::uword fdperseg(*fdperseg_np.data<arma::uword>());
    arma::uword radius(*radius_np.data<arma::uword>());
    arma::uword period_max(*period_max_np.data<arma::uword>());
    oT          threshold(*threshold_np.data<oT>());


    common::Logger logger(spdlog::stdout_color_mt("dsp"));
    Pipeline pipeline(logger);
    Handler data_handler(logger, &pipeline);
    Producer<iT> producer(logger, &data_handler, filename_in, period);

    auto fmt_data = producer.get_fmt();
    std::cout << "Input:\n"
              << "  size: (" << fmt_data.n_rows << "," << fmt_data.n_cols << "," << fmt_data.n_slices << ")\n"
              << "  period: " << period << "\n"
              << "------------------------------\n"
              << "Filter params:\n"
              << "  fdskip:     " << fdskip << "\n"
              << "  fdperseg:   " << fdperseg << "\n"
              << "  radius:     " << radius << "\n"
              << "  period_max: " << period_max << "\n"
              << "  threshold:  " << threshold << "\n"
              << "  b:\n"; b.t().print();
    std::cout << "  a:\n"; a.t().print();
    std::cout << "------------------------------\n";

    arma::SizeCube fmt_in(1, fmt_data.n_cols, fmt_data.n_slices);
    auto source_filter = new filter::source<iT, oT>(logger, &data_handler, common::data::type::us);
    source_filter->set_chunk_size(fmt_in.n_rows, fmt_in.n_cols, fmt_in.n_slices);
    pipeline.add_filter(std::unique_ptr<Filter>(source_filter));

    arma::SizeCube fmt_buffer(fdskip, fmt_in.n_cols, fmt_in.n_slices);
    auto buffer_filter = new filter::buffer<oT>(logger);
    pipeline.add_filter(std::unique_ptr<Filter>(buffer_filter));

    auto iir_filter = new filter::iir<oT, double>(logger, fmt_in.n_cols * fmt_in.n_slices, b, a);
    pipeline.add_filter(std::unique_ptr<Filter>(iir_filter));

    arma::SizeCube fmt_fhr(fdperseg, fmt_in.n_cols, fmt_in.n_slices);
    auto roll_filter = new filter::roll<oT>(logger, 1);
    pipeline.add_filter(std::unique_ptr<Filter>(roll_filter));

    arma::SizeCube fmt_out(1, fmt_in.n_cols, fmt_in.n_slices);
    auto fhr_filter = new filter::fhr<oT, oT, oT>(logger, radius, period_max, threshold);
    pipeline.add_filter(std::unique_ptr<Filter>(fhr_filter));

    auto sink_filter_0 = new NpySink<oT>(logger, fmt_data);
    pipeline.add_filter(std::unique_ptr<Filter>(sink_filter_0));

    auto sink_filter_1 = new NpySink<oT>(logger, fmt_data);
    pipeline.add_filter(std::unique_ptr<Filter>(sink_filter_1));

    pipeline.link<oT>(source_filter, buffer_filter, fmt_in);
    pipeline.link<oT>(buffer_filter, iir_filter, fmt_buffer);
    pipeline.link<oT>(iir_filter, roll_filter, fmt_buffer);
    pipeline.link<oT>(roll_filter, fhr_filter, fmt_fhr);
    pipeline.link<oT>(fhr_filter, sink_filter_0, fmt_out);
    pipeline.link<oT>(fhr_filter, sink_filter_1, fmt_out);

    std::thread pipeline_th(&Pipeline::run, &pipeline);
    std::thread producer_th(&Producer<iT>::run, &producer);

    producer_th.join();
    pipeline_th.join();

    sink_filter_0->dump("fhr_" + filename_out);
    sink_filter_1->dump("corr_" + filename_out);

    pipeline.print_stats();

    return 0;
}
