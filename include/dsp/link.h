#pragma once

#include <string>
#include <memory>
#include <deque>
#include <algorithm>

#include <armadillo>

#include "common/log.h"

#include "dsp_error.h"
#include "filter.h"

namespace dsp {

template<typename T>
using Chunk = arma::Cube<T>;

class LinkInterface
{
public:
    LinkInterface(Filter * src, Filter * dst, arma::SizeCube& format):
        src_(src), dst_(dst), format_(format) {}
    virtual ~LinkInterface() = default;

    void link(Filter * src, Filter * dst)
    {
        src->add_output(this);
        dst->add_input(this);
    }

    const arma::SizeCube& format()  const {return format_;}

    void eof_reached()  {eof_ = 1;}
    void reset_eof()    {eof_ = 0;}
    bool eof() const    {return eof_;}


protected:
    Filter * const src_;
    Filter * const dst_;
    const arma::SizeCube  format_;

    int eof_ = 0;
};

template<typename T>
struct Pad
{
    using type = T;
    std::string     name;
};

/* TODO: add a pool of chunk to avoid oom <01-04-20, cneyton> */
template<typename T>
class Link: public LinkInterface
{
public:
    using elem_type = std::shared_ptr<Chunk<T>>;

    Link(arma::SizeCube& format): LinkInterface(nullptr, nullptr, format) {}
    Link(Filter * src, Filter * dst, arma::SizeCube& format): LinkInterface(src, dst, format)
    {
        link(src, dst);
    }

    virtual ~Link() = default;

    int push(elem_type chunk)
    {
        if (arma::size(*chunk) != format_)
            throw dsp_error(Errc::invalid_chunk_format);

        chunk_queue_.emplace_back(chunk);

        dst_->set_ready();
        return 0;
    }

    int pop(elem_type& chunk)
    {
        if (chunk_queue_.empty()) {
            if (!eof_)
                src_->set_ready();
            return 0;
        }
        chunk = chunk_queue_.front();
        chunk_queue_.pop_front();
        return 1;
    }

    elem_type front() const
    {
        return chunk_queue_.front();
    }

    void pop()
    {
        chunk_queue_.pop_front();
    }

    template<class... Args>
    elem_type make_chunk(Args&&... args)
    {
        return std::make_shared<Chunk<T>>(std::forward<Args>(args)...);
    }

    arma::uword  size() const {return chunk_queue_.size();}
    bool        empty() const {return chunk_queue_.empty();}

private:
    std::deque<elem_type> chunk_queue_;
    //Pad<T> * src_pad_ = nullptr;
    //Pad<T> * dst_pad_ = nullptr;
};

} /* namespace dsp */
