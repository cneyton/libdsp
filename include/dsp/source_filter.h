#pragma once

#include "common/log.h"
#include "common/data.h"
#include "filter.h"
#include "link.h"
#include "exception.h"

namespace dsp::filter
{

template<typename T1, typename T2>
class source: public Filter
{
public:
    source(common::Logger logger, common::data::Handler * h,
           std::string_view type, arma::SizeCube& fmt):
        Log(logger), Filter(logger, "source"), data_handler_(h),
        type_(type), format_(fmt)
    {
        source_ = true;
    }

    virtual int activate()
    {
        log_debug(logger_, "filter {} activated", name_);

        Link<T2> * output = dynamic_cast<Link<T2>*>(outputs_.at(0));

        std::vector<std::string> data;
        switch (data_handler_->pop_chunk(type_, format_.n_rows, data)) {
        case 0: return 0;
        case 1: break;
        case 2: output->eof_reached(); return 1;
        default:
            throw filter_error("wrong enum, you should not be here");
        }

        if (!data_handler_->pop_chunk(type_, format_.n_rows, data)) {
            return 0;
        }

        auto chunk = std::make_shared<Chunk<T2>>(format_);

        arma::uword i = 0;
        for(auto& buf: data) {
            fill_frame(*chunk, buf, i++);
        }

        if (verbose_)
            chunk->print();

        output->push(chunk);
        return 1;
    }

private:
    common::data::Handler * data_handler_;
    std::string             type_;
    arma::SizeCube          format_;

    static void fill_frame(Chunk<T2>& chunk, const std::string& buf, const arma::uword frame_nb);
};

template<typename T1, typename T2>
inline
void source<T1, T2>::fill_frame(Chunk<T2>& chunk, const std::string& buf, const arma::uword frame_nb)
{
    size_t expected_size = chunk.n_cols * chunk.n_slices * sizeof(T1);
    if (buf.size() != expected_size) {
        throw filter_error(fmt::format("buffer size ({}) doesn't match expected size ({})",
                                       buf.size(), expected_size));
    }

    auto it = buf.cbegin();
    arma::uword j = 0, k = 0;
    arma::uword n = 0;
    while(it != buf.cend())
    {
        auto x = reinterpret_cast<const T1*>(&*it);
        it += sizeof(*x);
        chunk.at(frame_nb, j, k) = T2(*x);
        n++;
        j = n % chunk.n_cols;
        k = n / chunk.n_cols;
    }
}

template<>
inline
void source<common::data::us::IQ<int16_t>, arma::cx_double>::fill_frame(Chunk<arma::cx_double>& chunk,
                                                                        const std::string& buf,
                                                                        const arma::uword frame_nb)
{
    size_t expected_size = chunk.n_cols * chunk.n_slices * sizeof(common::data::us::IQ<int16_t>);
    if (buf.size() != expected_size) {
        throw filter_error(fmt::format("buffer size ({}) doesn't match expected size ({})",
                                       buf.size(), expected_size));
    }

    auto it = buf.cbegin();
    arma::uword j = 0, k = 0;
    arma::uword n = 0;
    while(it != buf.cend())
    {
        auto x = reinterpret_cast<const common::data::us::IQ<int16_t>*>(&*it);
        it += sizeof(*x);
        chunk.at(frame_nb, j, k) = arma::cx_double(x->i, x->q);
        n++;
        j = n % chunk.n_cols;
        k = n / chunk.n_cols;
    }
}

template<>
inline
void source<common::data::us::IQ<int16_t>, arma::cx_float>::fill_frame(Chunk<arma::cx_float>& chunk,
                                                                       const std::string& buf,
                                                                       const arma::uword frame_nb)
{
    size_t expected_size = chunk.n_cols * chunk.n_slices * sizeof(common::data::us::IQ<int16_t>);
    if (buf.size() != expected_size) {
        throw filter_error(fmt::format("buffer size ({}) doesn't match expected size ({})",
                                       buf.size(), expected_size));
    }

    auto it = buf.cbegin();
    arma::uword j = 0, k = 0;
    arma::uword n = 0;
    while(it != buf.cend())
    {
        auto x = reinterpret_cast<const common::data::us::IQ<int16_t>*>(&*it);
        it += sizeof(*x);
        chunk.at(frame_nb, j, k) = arma::cx_float(x->i, x->q);
        n++;
        j = n % chunk.n_cols;
        k = n / chunk.n_cols;
    }
}

} /* namespace dsp::filter */
