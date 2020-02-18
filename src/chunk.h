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

    int fill_frame(common::data::ByteBuffer& buf, uint16_t frame_nb)
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
};

#endif /* CHUNK_H */
