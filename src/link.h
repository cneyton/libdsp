#ifndef LINK_H
#define LINK_H

#include <memory>
#include <queue>

#include "common/log.h"

#include "filter.h"
#include "chunk.h"

class LinkInterface: public common::Log
{
public:
    LinkInterface(common::Logger logger, Filter * src, Filter * dst):
        Log(logger), src_(src), dst_(dst) {}
    virtual ~LinkInterface() {}

protected:
    Filter * src_ = nullptr;
    Filter * dst_ = nullptr;
};

template<typename T>
class Link : public LinkInterface
{
public:
    Link(common::Logger logger, Filter * src, Filter * dst):
        LinkInterface(logger, src, dst), chunk_queue_()
    {
        link(src, dst);
    }

    virtual ~Link() {}

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

    int push(std::shared_ptr<Chunk<T>> chunk)
    {
        chunk_queue_.emplace(chunk);
        dst_->set_ready();
        return 0;
    }

    int pop(std::shared_ptr<Chunk<T>>& chunk)
    {
        if (chunk_queue_.empty())
            return 0;
        chunk = chunk_queue_.front();
        chunk_queue_.pop();
        return 1;
    }

private:
    std::queue<std::shared_ptr<Chunk<T>>> chunk_queue_;
};

#endif /* LINK_H */
