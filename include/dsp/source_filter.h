#pragma once

#include <mutex>
#include <queue>

#include "common/log.h"

#include "exception.h"
#include "filter.h"
#include "link.h"
#include "type.h"

namespace dsp {

struct SourceItfc
{
    /*
     * Be careful when changing format. The pipeline must be cleared afterward.
     */
    virtual void set_format(arma::SizeCube& fmt)    = 0;
    virtual void push_frame(std::string_view frame) = 0;
    virtual void eof() = 0;
};

namespace filter {

template<typename T1, typename T2>
class source: public Filter, public SourceItfc
{
public:
    source(common::Logger logger, std::string_view name, arma::SizeCube& fmt):
        Filter(logger, name),
        format_(fmt),
        expected_frame_size_(expected_frame_size())
    {
    }

    int activate() override
    {
        log_debug(logger_, "filter {} activated", name_);

        Link<T2> * output = dynamic_cast<Link<T2>*>(outputs_.at(0));
        typename Link<T2>::elem_type front;

        {
            std::unique_lock<std::mutex> lk(mutex_);
            if (queue_.empty()) {
                if (eof_)
                    output->eof_reached();
                return 0;
            }
            front = std::move(queue_.front());
            queue_.pop();
        }

        if (verbose_)
            front->print();

        output->push(front);

        return 1;
    }

    void reset() override
    {
        std::unique_lock<std::mutex> lk(mutex_);
        while (!queue_.empty())
            queue_.pop();
        frame_nb_ = 0;
        eof_ = false;
    }

    void set_format(arma::SizeCube&) override
    {
        //format_ = fmt;
        expected_frame_size_ = expected_frame_size();
    }

    void push_frame(std::string_view frame) override
    {
        if (frame_nb_ == 0)
            // chunk_ will be overwritten if not null
            chunk_ = std::make_unique<Chunk<T2>>(format_);

        if (frame.size() != expected_frame_size_) {
            log_warn(logger_, "frame size ({}) doesn't match expected size ({}), pushing invalid frame",
                                           frame.size(), expected_frame_size_);
            std::string invalid_frame(expected_frame_size_, 0);
            fill_frame(invalid_frame, frame_nb_);
        } else {
            fill_frame(frame, frame_nb_);
        }

        if (++frame_nb_ < format_.n_rows)
            return;

        std::unique_lock<std::mutex> lk(mutex_);
        queue_.push(std::move(chunk_));
        frame_nb_ = 0;
        set_ready();
    }

    void eof() override
    {
        std::unique_lock<std::mutex> lk(mutex_);
        eof_ = true;
        set_ready();
    }

private:
    arma::SizeCube  format_;
    arma::uword     frame_nb_ = 0;
    bool            eof_ = false;
    size_t          expected_frame_size_;

    using elem_type = std::unique_ptr<Chunk<T2>>;
    elem_type             chunk_ = nullptr;
    std::queue<elem_type> queue_;
    std::mutex            mutex_;

    size_t expected_frame_size() {return format_.n_cols * format_.n_slices * sizeof(T1);};
    void   fill_frame(std::string_view buf, const arma::uword frame_nb);
};

template<typename T1, typename T2>
inline
void source<T1, T2>::fill_frame(std::string_view buf, const arma::uword frame_nb)
{
    auto it = buf.cbegin();
    arma::uword j = 0, k = 0;
    arma::uword n = 0;
    while(it != buf.cend()) {
        auto x = reinterpret_cast<const T1*>(&*it);
        it += sizeof(*x);
        chunk_->at(frame_nb, j, k) = T2(*x);
        n++;
        j = n % chunk_->n_cols;
        k = n / chunk_->n_cols;
    }
}

template<>
inline
void source<IQ<int16_t>, arma::cx_double>::fill_frame(std::string_view buf,
                                                      const arma::uword frame_nb)
{
    auto it = buf.cbegin();
    arma::uword j = 0, k = 0;
    arma::uword n = 0;
    while(it != buf.cend()) {
        auto x = reinterpret_cast<const IQ<int16_t>*>(&*it);
        it += sizeof(*x);
        chunk_->at(frame_nb, j, k) = arma::cx_double(x->i, x->q);
        n++;
        j = n % chunk_->n_cols;
        k = n / chunk_->n_cols;
    }
}

template<>
inline
void source<IQ<int16_t>, arma::cx_float>::fill_frame(std::string_view buf,
                                                     const arma::uword frame_nb)
{
    auto it = buf.cbegin();
    arma::uword j = 0, k = 0;
    arma::uword n = 0;
    while(it != buf.cend()) {
        auto x = reinterpret_cast<const IQ<int16_t>*>(&*it);
        it += sizeof(*x);
        chunk_->at(frame_nb, j, k) = arma::cx_float(x->i, x->q);
        n++;
        j = n % chunk_->n_cols;
        k = n / chunk_->n_cols;
    }
}

} /* namespace filter */
} /* namespace dsp */