#include <memory>

#include "common/log.h"

#include "link.h"

#include "spdlog/sinks/stdout_color_sinks.h"
common::Logger logger(spdlog::stdout_color_mt("dsp"));

int main()
{
    arma::SizeCube format(1,5,2);
    Link<double>  link(logger, format);
    for (auto i = 0; i < 10; ++i) {
        auto in_chunk = std::make_shared<Chunk<double>>(format, arma::fill::ones);
        *in_chunk *= i;
        link.push(in_chunk);
    }

    auto out_chunk = link.head_chunk(5);
    out_chunk.print();
    link.pop_head(2);
    out_chunk = link.head_chunk(5);
    out_chunk.print();

    return 0;
}
