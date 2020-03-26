#include "test_utils.h"

#include "iir_filter.h"
#include "roll_filter.h"
#include "fd_filter.h"

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

    common::Logger logger(spdlog::stdout_color_mt("dsp"));

    Pipeline pipeline(logger);
    Handler data_handler(logger, &pipeline);
    Producer<iT> producer(logger, &data_handler, filename_in);

    auto fmt_data = producer.get_fmt();
    std::cout << "Input:\n"
              << "  size: (" << fmt_data.n_rows << "," << fmt_data.n_cols << "," << fmt_data.n_slices << ")\n"
              << "------------------------------\n";

    cnpy::NpyArray nskip_np  = cnpy::npz_load(filename_params, "nskip");
    cnpy::NpyArray n_in_np   = cnpy::npz_load(filename_params, "n_in");
    cnpy::NpyArray n_out_np  = cnpy::npz_load(filename_params, "n_out");

    arma::uword nskip(*nskip_np.data<arma::uword>());
    arma::uword n_in(*n_in_np.data<arma::uword>());
    arma::uword n_out(*n_out_np.data<arma::uword>());

    std::cout << "Filter params:\n"
              << "   n_in:  " << n_in << "\n"
              << "   n_out: " << n_in << "\n"
              << "   nskip: " << nskip << "\n"
              << "------------------------------\n";

    arma::SizeCube fmt_in(n_in, fmt_data.n_cols, fmt_data.n_slices);
    auto source_filter = new filter::source<iT, oT>(logger, &data_handler, common::data::type::us);
    source_filter->set_chunk_size(fmt_in.n_rows, fmt_in.n_cols, fmt_in.n_slices);
    pipeline.add_filter(std::unique_ptr<Filter>(source_filter));

    arma::SizeCube fmt_out(n_out, fmt_in.n_cols, fmt_in.n_slices);
    auto roll_filter = new filter::roll<oT>(logger, nskip);
    pipeline.add_filter(std::unique_ptr<Filter>(roll_filter));

    auto sink_filter = new NpySink<oT>(logger, fmt_data);
    pipeline.add_filter(std::unique_ptr<Filter>(sink_filter));

    pipeline.link<oT>(source_filter, roll_filter, fmt_in);
    pipeline.link<oT>(roll_filter, sink_filter, fmt_out);

    std::thread pipeline_th(&Pipeline::run, &pipeline);
    std::thread producer_th(&Producer<iT>::run, &producer);

    producer_th.join();
    pipeline_th.join();

    sink_filter->dump(filename_out);

    pipeline.print_stats();

    return 0;
}
