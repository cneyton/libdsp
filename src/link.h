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
    int push(std::shared_ptr<Chunk> chunk);
    int pop(std::shared_ptr<Chunk>& chunk);

private:
    Filter * src_ = nullptr;
    Filter * dst_ = nullptr;
    std::queue<std::shared_ptr<Chunk>> chunk_queue_;
};

#endif /* LINK_H */
