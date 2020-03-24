#include "test_utils.h"
#include "source_filter.h"

#include "spdlog/common.h"
#include "spdlog/sinks/stdout_color_sinks.h"

using iT = double;
using oT = double;


/* TODO: we should test different type of input/output <20-02-20, cneyton> */
int main(int argc, char * argv[])
{
    if (argc != 3)
        return -1;

    std::string filename_in(argv[1]);
    std::string filename_out(argv[2]);

    common::Logger logger(spdlog::stdout_color_mt("dsp"));

    Pipeline pipeline(logger);
    Handler data_handler(logger, &pipeline);
    Producer<iT> producer(logger, &data_handler, filename_in);

    auto fmt_data = producer.get_fmt();
    arma::SizeCube fmt_in(2, fmt_data.n_cols, fmt_data.n_slices);
    std::cout << "Input:\n"
              << "  type: " << typeid(iT).name() << "\n"
              << "  chunk size: (" << fmt_in.n_rows << "," << fmt_in.n_cols << "," << fmt_data.n_slices << ")\n"
              << "  nb frames total: " << fmt_data.n_rows << "\n"
              << "------------------------------\n";

    auto source_filter = new filter::source<iT, oT>(logger, &data_handler, common::data::type::us);
    source_filter->set_chunk_size(fmt_in.n_rows, fmt_in.n_cols, fmt_in.n_slices);
    pipeline.add_filter(std::unique_ptr<Filter>(source_filter));

    auto sink_filter = new NpySink<oT>(logger, fmt_data);
    pipeline.add_filter(std::unique_ptr<Filter>(sink_filter));

    pipeline.link<oT>(source_filter, sink_filter, fmt_in);

    std::thread pipeline_th(&Pipeline::run, &pipeline);
    std::thread producer_th(&Producer<double>::run, &producer);

    producer_th.join();
    pipeline.stop();
    pipeline_th.join();

    sink_filter->dump(filename_out);

    // report
    pipeline.print_stats();

    return 0;
}
