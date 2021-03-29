#pragma once

#include <deque>

#include "common/log.h"
#include "filter.h"
#include "link.h"

namespace dsp::filter {

/**
 * @brief Builds the feature matrice for the quality index
 *
 * @tparam T1 IQ chunk type
 * @tparam T2 correlation coefficient type
 * @tparam T3 feature matrix type
 */
template<typename T1, typename T2, typename T3 = double>
class QIFeatureFilter: public Filter
{
public:
    QIFeatureFilter(common::Logger logger, std::string_view name = "qi_feat_extractor"):
        Filter(logger, name)
    {
        Pad iq  {.name = "iq" , .format = Format()};
        Pad cor {.name = "cor", .format = Format()};
        Pad out {.name = "out", .format = Format()};
        input_pads_.insert({iq.name, iq});
        input_pads_.insert({cor.name, cor});
        output_pads_.insert({out.name, out});
    }

    int activate() override
    {
        log_debug(logger_, "{} filter activated", name_);

        auto input_iq  = dynamic_cast<Link<T1>*>(inputs_.at("iq"));
        auto input_cor = dynamic_cast<Link<T2>*>(inputs_.at("cor"));
        auto output    = dynamic_cast<Link<T3>*>(outputs_.at("out"));

        if (!iq_received_ && !input_iq->pop(chunk_iq_)) {
            if (input_iq->eof())
                output->eof_reached();
            return 0;
        }
        iq_received_ = true;

        // we have the iq waveform, wait for the corcoeff
        if (!cor_received_ && !input_cor->pop(chunk_cor_)) {
            if (input_cor->eof())
                output->eof_reached();
            return 0;
        }
        cor_received_ = true;

        // if one input is missing, wait...
        if (!iq_received_ || !cor_received_)
            return 0;

        iq_received_  = false;
        cor_received_ = false;
        const auto fmt_out = output->format();
        auto chunk_out = std::make_shared<Chunk<T3>>(chunk_iq_->timestamp, chunk_iq_->sample_period,
                                                     fmt_out);

        // computes the power
        for (arma::uword k = 0; k < fmt_out.n_slices; ++k) {
            chunk_out->slice(k).row(0) = arma::stddev(chunk_iq_->slice(k));
        }

        // add the correlation as the last feature
        chunk_out->row(1) = chunk_cor_->row(0);

        output->push(chunk_out);
        return 1;
    }

    void reset() override
    {
        /* TODO: todo <24-03-21, cneyton> */
    }

    Contract negotiate_format() override
    {
        auto fmt_iq  = input_pads_["iq"].format;
        auto fmt_cor = input_pads_["cor"].format;
        auto fmt_out = output_pads_["out"].format;

        if ((fmt_iq.n_cols   != fmt_cor.n_cols)   || (fmt_iq.n_cols   != fmt_out.n_cols) ||
            (fmt_iq.n_slices != fmt_cor.n_slices) || (fmt_iq.n_slices != fmt_out.n_slices) ||
            (fmt_cor.n_rows  != 1) || (fmt_out.n_rows != 2)) {
            return Contract::unsupported_format;
        }

        return Contract::supported_format;
    }

private:
    // we can use a map here if there is more input
    bool cor_received_ = false;
    bool iq_received_  = false;
    std::shared_ptr<Chunk<T1>> chunk_iq_;
    std::shared_ptr<Chunk<T2>> chunk_cor_;
};

} /* namespace dsp::filter */
