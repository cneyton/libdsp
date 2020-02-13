#ifndef CHUNK_H
#define CHUNK_H

#include <armadillo>
#include "common/data.h"
#include "common/log.h"

class Chunk: public common::Log, public arma::cx_cube
{
public:
    Chunk(common::Logger logger);
    Chunk(common::Logger logger, uint16_t n_frames, uint16_t n_samples, uint16_t n_slots);
    Chunk(common::Logger logger, const arma::SizeCube& size);
    virtual ~Chunk();

    int fill_frame(common::data::ByteBuffer& buf, uint16_t frame_nb);

private:
};

#endif /* end of include guard: CHUNK_H */
