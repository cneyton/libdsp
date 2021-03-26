#include "test_utils.h"

#include "dsp/qi_feature_filter.h"

#include "spdlog/common.h"
#include "spdlog/sinks/stdout_color_sinks.h"

using T1 = double;
using T2 = double;

int main(int argc, char * argv[])
{
    if (argc != 4)
        return -1;

    std::string filename_iq(argv[1]);
    std::string filename_cor(argv[2]);
    std::string filename_out(argv[3]);

    common::Logger logger(spdlog::stdout_color_mt("dsp"));
    logger->set_level(spdlog::level::info);

    Pipeline pipeline(logger);

    auto source_filter_iq = std::make_unique<NpySource<T1>>(logger, filename_iq, 1, "iq source");
    auto fmt_data = source_filter_iq->get_fmt();
    auto source_iq = pipeline.add_filter(std::move(source_filter_iq));

    auto source_filter_cor = std::make_unique<NpySource<T2>>(logger, filename_cor, 1, "cor source");
    auto source_cor = pipeline.add_filter(std::move(source_filter_cor));

    auto extractor = std::make_unique<filter::QIFeatureFilter<T1, T2>>(logger);
    auto extractor_h = pipeline.add_filter(std::move(extractor));

    auto sink_filter = std::make_unique<NpySink<T2>>(logger, fmt_data);
    auto sink_p = sink_filter.get();
    auto sink_h = pipeline.add_filter(std::move(sink_filter));

    pipeline.link<T1>(source_iq, "out", extractor_h, "iq");
    pipeline.link<T2>(source_cor, "out", extractor_h, "cor");
    pipeline.link<T2>(extractor_h, "out", sink_h, "in");

    arma::uword n_iq = 30;
    arma::uword n_feat = 2;
    Format fmt_iq  { n_iq  , fmt_data.n_cols, fmt_data.n_slices };
    Format fmt_cor { 1     , fmt_data.n_cols, fmt_data.n_slices };
    Format fmt_out { n_feat, fmt_iq.n_cols, fmt_iq.n_slices };
    source_iq->set_output_format(fmt_iq, "out");
    source_cor->set_output_format(fmt_cor, "out");
    extractor_h->set_input_format(fmt_iq, "iq");
    extractor_h->set_input_format(fmt_cor, "cor");
    extractor_h->set_output_format(fmt_out, "out");
    sink_h->set_input_format(fmt_out, "in");
    if (pipeline.negotiate_format() != Contract::supported_format)
        throw dsp_error(Errc::format_negotiation_failed);

    sink_p->dump(filename_out);

    pipeline.print_stats();

    return 0;
}

