#include <memory>

#include "common/log.h"

#include "dsp/link.h"

#include "spdlog/sinks/stdout_color_sinks.h"
common::Logger logger(spdlog::stdout_color_mt("dsp"));

using namespace dsp;

int main()
{
    arma::SizeCube format(1,5,2);
    Link<double>  link(format);
    for (auto i = 0; i < 10; ++i) {
        auto in_chunk = std::make_shared<Chunk<double>>(format, arma::fill::ones);
        *in_chunk *= i;
        link.push(in_chunk);
    }

    return 0;
}
