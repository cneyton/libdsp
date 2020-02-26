#ifndef LINK_H
#define LINK_H

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
    LinkInterface(common::Logger logger, Filter * src, Filter * dst):
        Log(logger), src_(src), dst_(dst) {}
    virtual ~LinkInterface() {}

    int link(Filter * src, Filter * dst)
    {
        src_ = src;
        dst_ = dst;

        int ret;
        ret = src->add_output(*this);
        common_die_zero(logger_, ret, -1, "failed to add link to src");

        ret = dst->add_input(*this);
        common_die_zero(logger_, ret, -2, "failed to add link to dst");

        return 0;
    }

protected:
    Filter * src_ = nullptr;
    Filter * dst_ = nullptr;
};

template<typename T>
class Link : public LinkInterface
{
public:

    using elem_type = std::shared_ptr<Chunk<T>>;

    Link(common::Logger logger): LinkInterface(logger, nullptr, nullptr) {}

    Link(common::Logger logger, Filter * src, Filter * dst): LinkInterface(logger, src, dst)
    {
        link(src, dst);
    }

    virtual ~Link() {}

    int push(elem_type chunk)
    {
        /* TODO: check chunk size <25-02-20, cneyton> */
        chunk_queue_.emplace_back(chunk);
        common_die_null(logger_, dst_, -1, "dst nullptr");
        dst_->set_ready();
        return 0;
    }

    int front(elem_type& chunk) const
    {
        if (chunk_queue_.empty())
            return -1;
        chunk = chunk_queue_.front();
        return 0;
    }

    int pop()
    {
        if (chunk_queue_.empty())
            return -1;
        chunk_queue_.pop_front();
        return 0;
    }

    int head(std::vector<elem_type>& head, const arma::uword n) const
    {
        if (chunk_queue_.size() < n)
            return -1;

        head.clear();
        head.reserve(n);
        std::for_each(chunk_queue_.cbegin(), chunk_queue_.cbegin() + n,
                      [&](auto& chunk){head.push_back(chunk);});
        return 0;
    }

    int pop_head(const arma::uword n)
    {
        if (chunk_queue_.size() < n)
            return -1;

        for (arma::uword i = 0; i < n; ++i)
            chunk_queue_.pop_front();
    }

    arma::uword  size() const {return chunk_queue_.size();}
    bool        empty() const {return chunk_queue_.empty();}

private:
    std::deque<elem_type> chunk_queue_;
};

#endif /* LINK_H */
