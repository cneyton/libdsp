#pragma once

#include <filesystem>

#include "mlpack/core.hpp"
#include "mlpack/methods/random_forest/random_forest.hpp"

#include "common/log.h"
#include "filter.h"
#include "link.h"

namespace dsp::filter {

/* TODO: use template for model <24-03-21, cneyton> */
template<typename T = double>
class Classifier: public Filter
{
public:
    Classifier(common::Logger logger, std::string_view name = "classifier"):
        Filter(logger, name)
    {
        Pad in  {.name = "in" , .format = Format()};
        Pad out {.name = "out", .format = Format()};
        input_pads_.insert({in.name, in});
        output_pads_.insert({out.name, out});
    }

    int activate() override
    {
        log_debug(logger_, "{} filter activated", name_);

        auto input    = dynamic_cast<Link<T>*>(inputs_.at("in"));
        auto output   = dynamic_cast<Link<T>*>(outputs_.at("out"));
        auto chunk_in = std::make_shared<Chunk<T>>();

        if (!input->pop(chunk_in)) {
            if (input->eof())
                output->eof_reached();
            return 0;
        }

        const auto fmt_out = output->format();
        auto chunk_out = std::make_shared<Chunk<T>>(chunk_in->timestamp, chunk_in->sample_period,
                                                    fmt_out);

        arma::Row<size_t> predictions;
        for (arma::uword k = 0; k < fmt_out.n_slices; ++k) {
            rf_.Classify(chunk_in->slice(k), predictions, chunk_out->slice(k));
        }

        output->push(chunk_out);
    }

    void reset() override
    {
        // nothing to do
    }

    Contract negotiate_format() override
    {
        auto fmt_in  = input_pads_["in"].format;
        auto fmt_out = output_pads_["out"].format;

        if (fmt_in.n_cols   != fmt_out.n_cols ||
            fmt_in.n_slices != fmt_out.n_slices ||
            fmt_out.n_rows  != 1 ) {
            return Contract::unsupported_format;
        }

        return Contract::supported_format;
    }

    void load_model(const std::filesystem::path& filename, const std::string& model_name)
    {
        mlpack::data::Load(filename, model_name, rf_, true, mlpack::data::format::xml);
    }

private:
    mlpack::tree::RandomForest<mlpack::tree::GiniGain> rf_;
};

} /* namespace dsp::filter */
