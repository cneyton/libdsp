#pragma once

#include <string>
#include <memory>
#include <deque>
#include <algorithm>

#include <armadillo>

#include "common/log.h"

#include "dsp_error.h"
#include "filter.h"
#include "format.h"

namespace dsp {

template<typename T>
using Chunk = arma::Cube<T>;

class LinkInterface
{
public:
    LinkInterface(Filter * src, const std::string& src_pad_name,
                  Filter * dst, const std::string& dst_pad_name):
        src_(src), dst_(dst),
        src_pad_name_(src_pad_name), dst_pad_name_(dst_pad_name)
    {
        src->add_output(this, src_pad_name);
        dst->add_input(this,  dst_pad_name);
    }

    virtual ~LinkInterface() = default;

    const Format& format()  const {return format_;}

    Contract negotiate_format()
    {
        auto src_fmt = src_->get_output_format(src_pad_name_);
        auto dst_fmt = dst_->get_input_format(dst_pad_name_);

        if (src_fmt != dst_fmt)
            return Contract::supported_format;

        format_ = src_fmt;
        return Contract::supported_format;
    }

    void eof_reached()  {eof_ = 1;}
    void reset_eof()    {eof_ = 0;}
    bool eof() const    {return eof_;}


protected:
    Filter * const src_;
    Filter * const dst_;
    std::string src_pad_name_;
    std::string dst_pad_name_;
    Format  format_;

    int eof_ = 0;
};


/* TODO: add a pool of chunk to avoid oom <01-04-20, cneyton> */
template<typename T>
class Link: public LinkInterface
{
public:
    using elem_type = std::shared_ptr<Chunk<T>>;

    Link(): LinkInterface(nullptr, "", nullptr, "") {}
    Link(Filter * src, const std::string& src_pad_name,
         Filter * dst, const std::string& dst_pad_name):
        LinkInterface(src, src_pad_name, dst, dst_pad_name) {}

    virtual ~Link() = default;

    int push(elem_type chunk)
    {
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
};

} /* namespace dsp */
