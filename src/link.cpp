#include "link.h"


Link::Link(common::Logger logger, Filter * src, Filter * dst): Log(logger), src_(src), dst_(dst),
    chunk_queue_()
{
    link(src, dst);
}

Link::~Link()
{
}

int Link::link(Filter * src, Filter * dst)
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

int Link::push(Chunk&& chunk)
{
    chunk_queue_.push(std::move(chunk));
    dst_->set_ready();
    return 0;
}

int Link::pop(Chunk& chunk)
{
    chunk = chunk_queue_.front();
    chunk_queue_.pop();
    return 0;
}
