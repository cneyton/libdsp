#ifndef LINK_H
#define LINK_H

#include <string>
#include <memory>
#include <deque>
#include <algorithm>

#include <armadillo>

#include "common/log.h"

#include "filter.h"

template<typename T>
using Chunk = arma::Cube<T>;


class LinkInterface: public common::Log
{
public:
    LinkInterface(common::Logger logger, Filter * src, Filter * dst, arma::SizeCube& format):
        Log(logger), src_(src), dst_(dst), format_(format) {}
    virtual ~LinkInterface() {}

    int link(Filter * src, Filter * dst)
    {
        int ret;
        ret = src->add_output(*this);
        common_die_zero(logger_, ret, -1, "failed to add link to src");

        ret = dst->add_input(*this);
        common_die_zero(logger_, ret, -2, "failed to add link to dst");

        return 0;
    }

    void eof_reached()  {eof_ = 1;}
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

template<typename T>
class Link : public LinkInterface
{
public:

    using elem_type = std::shared_ptr<Chunk<T>>;

    Link(common::Logger logger, arma::SizeCube& format):
        LinkInterface(logger, nullptr, nullptr, format) {}

    Link(common::Logger logger, Filter * src, Filter * dst, arma::SizeCube& format):
        LinkInterface(logger, src, dst, format)
    {
        link(src, dst);
    }

    virtual ~Link() {}

    int push(elem_type chunk)
    {
        if (arma::size(*chunk) != format_)
            common_die(logger_, -1, "invalid chunk format");

        chunk_queue_.emplace_back(chunk);

        common_die_null(logger_, dst_, -1, "dst nullptr");
        dst_->set_ready();
        return 0;
    }

    int pop(elem_type& chunk)
    {
        if (chunk_queue_.empty()) {
            common_die_null(logger_, src_, -1, "src nullptr");
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

    int head(std::vector<elem_type>& frames, const arma::uword n) const
    {
        if (chunk_queue_.size() < n)
            return -1;

        frames.clear();
        frames.reserve(n);
        std::for_each(chunk_queue_.cbegin(), chunk_queue_.cbegin() + n,
                      [&](auto& chunk){frames.push_back(chunk);});
        return 0;
    }

    int pop_head(const arma::uword n)
    {
        if (chunk_queue_.size() < n)
            return -1;

        for (arma::uword i = 0; i < n; ++i)
            chunk_queue_.pop_front();

        return 0;
    }

    Chunk<T> head_chunk(const arma::uword n) const
    {
        auto chunk  = arma::Cube<T>(n, format_.n_cols, format_.n_slices);
        auto frames = std::vector<elem_type>();
        head(frames, n);

        for (arma::uword i = 0; i < n; ++i) {
            for (arma::uword j = 0; j < format_.n_cols; ++j) {
                for (arma::uword k = 0; k < format_.n_slices; ++k) {
                    chunk(i, j, k) = frames[i]->operator()(0, j, k);
                }
            }
        }
        return chunk;
    }

    arma::uword  size() const {return chunk_queue_.size();}
    bool        empty() const {return chunk_queue_.empty();}

    const arma::SizeCube& get_format()  const {return format_;}

private:
    std::deque<elem_type> chunk_queue_;
    //Pad<T> * src_pad_ = nullptr;
    //Pad<T> * dst_pad_ = nullptr;
};

#endif /* LINK_H */
