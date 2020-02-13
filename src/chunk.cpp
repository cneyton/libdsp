#include "chunk.h"
#include "common/log.h"

Chunk::Chunk(common::Logger logger): Log(logger), arma::cx_cube()
{
}

Chunk::Chunk(common::Logger logger, uint16_t n_frames, uint16_t n_samples, uint16_t n_slots) :
    Log(logger), arma::cx_cube(n_frames, n_samples, n_slots)
{
}

Chunk::Chunk(common::Logger logger, const arma::SizeCube& size):
    Log(logger), arma::cx_cube(size)
{
}

Chunk::~Chunk()
{
}

int Chunk::fill_frame(common::data::ByteBuffer& buf, uint16_t frame_nb)
{

    auto it = buf.cbegin();
    uint16_t j = 0, k = 0;
    uint32_t n = 0;
    int16_t I, Q;

    if(buf.size() != this->n_cols * this->n_slices * 2 * sizeof(I))
        common_die(logger_, -1, "invalid buffer size");

    while(it != buf.cend())
    {
        std::copy(it, it + sizeof(I), (uint8_t*)&I);
        it += sizeof(I);

        std::copy(it, it + sizeof(Q), (uint8_t*)&Q);
        it += sizeof(Q);

        this->at(frame_nb, j, k) = arma::cx_double(I, Q);
        n++;
        j = n % this->n_cols;
        k = n / this->n_cols;
    }
    return 0;
}
