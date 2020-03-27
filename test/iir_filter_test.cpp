#include "test_utils.h"

#include "iir_filter.h"

#include "spdlog/common.h"
#include "spdlog/sinks/stdout_color_sinks.h"

using iT = arma::cx_double;
using oT = arma::cx_double;

int main(int argc, char * argv[])
{
    if (argc != 4)
        return -1;

    std::string filename_in(argv[1]);
    std::string filename_out(argv[2]);
    std::string filename_params(argv[3]);

    cnpy::NpyArray n_frames_np= cnpy::npz_load(filename_params, "n_frames");
    cnpy::NpyArray period_np  = cnpy::npz_load(filename_params, "period");
    cnpy::NpyArray nskip_np   = cnpy::npz_load(filename_params, "nskip");
    cnpy::NpyArray a_np       = cnpy::npz_load(filename_params, "a");
    cnpy::NpyArray b_np       = cnpy::npz_load(filename_params, "b");

    arma::uword n_frames(*n_frames_np.data<arma::uword>());
    arma::uword period(*period_np.data<arma::uword>());
    arma::uword nskip(*nskip_np.data<arma::uword>());
    arma::vec b(b_np.data<double>(), b_np.shape.at(0));
    arma::vec a(a_np.data<double>(), a_np.shape.at(0));

    common::Logger logger(spdlog::stdout_color_mt("dsp"));

    Pipeline pipeline(logger);
    Handler data_handler(logger, &pipeline);
    Producer<iT> producer(logger, &data_handler, filename_in, period, n_frames);

    auto fmt_data = producer.get_fmt();
    arma::SizeCube fmt_in(nskip, fmt_data.n_cols, fmt_data.n_slices);
    std::cout << "Input:\n"
              << "  type: " << typeid(iT).name() << "\n"
              << "  chunk size: (" << fmt_in.n_rows << "," << fmt_in.n_cols << "," << fmt_data.n_slices << ")\n"
              << "  nb frames total: " << fmt_data.n_rows << "\n"
              << "------------------------------\n"
              << "Filter params:\n"
              << "   b:\n"; b.t().print();
    std::cout << "   a:\n"; a.t().print();
    std::cout << "------------------------------\n";

    auto source_filter = new filter::source<iT, oT>(logger, &data_handler, common::data::type::us);
    source_filter->set_chunk_size(fmt_in.n_rows, fmt_in.n_cols, fmt_in.n_slices);
    pipeline.add_filter(std::unique_ptr<Filter>(source_filter));

    auto iir_filter = new filter::iir<oT, double>(logger, fmt_in.n_cols * fmt_in.n_slices, b, a);
    pipeline.add_filter(std::unique_ptr<Filter>(iir_filter));

    auto sink_filter = new NpySink<oT>(logger, fmt_data);
    pipeline.add_filter(std::unique_ptr<Filter>(sink_filter));

    pipeline.link<oT>(source_filter, iir_filter, fmt_in);
    pipeline.link<oT>(iir_filter, sink_filter, fmt_in);

    std::thread pipeline_th(&Pipeline::run, &pipeline);
    std::thread producer_th(&Producer<iT>::run, &producer);

    producer_th.join();
    pipeline_th.join();

    sink_filter->dump(filename_out);

    pipeline.print_stats();

    return 0;
}
