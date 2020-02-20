#ifndef CHUNK_H
#define CHUNK_H

#include <armadillo>
#include "common/data.h"
#include "common/log.h"

template<typename T>
class Chunk: public common::Log, public arma::Cube<T>
{
public:
    Chunk(common::Logger logger): Log(logger), arma::Cube<T>() {}
    Chunk(common::Logger logger, const arma::SizeCube& size): Log(logger), arma::Cube<T>(size) {}
    virtual ~Chunk() {}
};

#endif /* CHUNK_H */
