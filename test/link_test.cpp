#include <memory>

#include "common/log.h"

#include "link.h"

#include "spdlog/sinks/stdout_color_sinks.h"
common::Logger logger(spdlog::stdout_color_mt("dsp"));

int main()
{
    Link<double>  link(logger);
    for (auto i = 0; i < 10; ++i) {
        auto in_chunk = std::make_shared<Chunk<double>>(arma::SizeCube(1, 3, 2), arma::fill::ones);
        *in_chunk *= i;
        link.push(in_chunk);
    }
    std::vector<std::shared_ptr<Chunk<double>>> head;
    if (link.head(head, 3)) {
        for (auto& chunk: head) {
            chunk->print();
        }
    }
    return 0;
}
