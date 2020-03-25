#include "test_utils.h"

#include "iir_filter.h"
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
    arma::uword nfft = std::stoi(argv[3]);

    common::Logger logger(spdlog::stdout_color_mt("dsp"));

    Pipeline pipeline(logger);
    Handler data_handler(logger, &pipeline);
    Producer<iT> producer(logger, &data_handler, filename_in);

    auto fmt_data = producer.get_fmt();
    arma::SizeCube fmt_in(nfft, fmt_data.n_cols, fmt_data.n_slices);
    std::cout << "Input:\n"
              << "  type: " << typeid(iT).name() << "\n"
              << "  chunk size: (" << fmt_in.n_rows << "," << fmt_in.n_cols << "," << fmt_data.n_slices << ")\n"
              << "  nb frames total: " << fmt_data.n_rows << "\n"
              << "------------------------------\n";

    auto source_filter = new filter::source<iT, oT>(logger, &data_handler, common::data::type::us);
    source_filter->set_chunk_size(fmt_in.n_rows, fmt_in.n_cols, fmt_in.n_slices);
    pipeline.add_filter(std::unique_ptr<Filter>(source_filter));

    auto sink_filter = new NpySink<double>(logger, fmt_data);
    pipeline.add_filter(std::unique_ptr<Filter>(sink_filter));

    arma::SizeCube fmt_out(1, fmt_in.n_cols, fmt_in.n_slices);
    auto fd_filter = new filter::fd<oT, double>(logger, nfft, arma::vec(nfft, arma::fill::ones));
    pipeline.add_filter(std::unique_ptr<Filter>(fd_filter));

    std::cout << "Filter params:\n"
              << "   nfft: " << nfft << "\n"
              << "------------------------------\n";

    pipeline.link<oT>(source_filter, fd_filter, fmt_in);
    pipeline.link<double>(fd_filter, sink_filter, fmt_out);

    std::thread pipeline_th(&Pipeline::run, &pipeline);
    std::thread producer_th(&Producer<iT>::run, &producer);

    producer_th.join();
    pipeline_th.join();

    sink_filter->dump(filename_out);

    pipeline.print_stats();

    return 0;
}

