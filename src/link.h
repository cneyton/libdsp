#ifndef LINK_H
#define LINK_H

#include <memory>
#include <queue>

#include "common/log.h"

#include "filter.h"
#include "chunk.h"


class Link : public common::Log
{
public:
    Link(common::Logger loggger, Filter * src, Filter * dst);
    virtual ~Link();

    int link(Filter * src, Filter * dst);
    int push(Chunk&& chunk);
    int pop(Chunk& chunk);

private:
    Filter * src_;
    Filter * dst_;
    std::queue<Chunk> chunk_queue_;
};

#endif /* LINK_H */
