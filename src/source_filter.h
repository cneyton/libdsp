#ifndef SOURCE_FILTER_H
#define SOURCE_FILTER_H

#include "common/log.h"
#include "common/data.h"
#include "filter.h"
#include "chunk.h"
#include "link.h"

namespace filter
{

template<typename T1, typename T2>
class source: public Filter, public common::data::Consumer
{
public:
    source(common::Logger logger, common::data::Handler * data_handler, common::data::type type):
        Log(logger), Filter(logger), common::data::Consumer(logger, data_handler), type_(type)
    {
        ready_ = true;
    }

    ~source() {}

    int set_chunk_size(uint16_t nb_frames, uint16_t nb_samples, uint16_t nb_slots)
    {
        nb_frames_  = nb_frames;
        nb_samples_ = nb_samples;
        nb_slots_   = nb_slots;
        return 0;
    };

    virtual int activate()
    {
        log_debug(logger_, "source {} activated", this->name_);

        int ret;
        std::vector<common::data::ByteBuffer> data;

        auto chunk_size = arma::SizeCube(nb_frames_, nb_samples_, nb_slots_);

        ret = pop_chunk(type_, chunk_size.n_rows, data);
        common_die_zero(logger_, ret, -1, "source {} failed to pop chunk", this->name_);

        auto chunk = std::make_shared<Chunk<T2>>(logger_, chunk_size);

        uint16_t i = 0;
        for(auto& buf: data)
        {
            ret = fill_frame(*chunk, buf, i);
            common_die_zero(logger_, ret, -2, "source {} failed to fill frame", this->name_);
            i++;
        }

        if (verbose_)
            chunk->print();

        Link<T2> * output = dynamic_cast<Link<T2>*>(outputs_.at(0));
        output->push(chunk);
        return 0;
    }

private:
    common::data::type type_;
    uint16_t nb_frames_;
    uint16_t nb_samples_;
    uint16_t nb_slots_;

    int fill_frame(Chunk<T2>& chunk, const common::data::ByteBuffer& buf,
                   const arma::uword frame_nb) const;
};

/* TODO: move somewhere else <20-02-20, cneyton> */
template<typename T>
struct iq
{
    using elem_type = T;
    T i; T q;
};

template<typename T1, typename T2>
inline
int source<T1, T2>::fill_frame(Chunk<T2>& chunk, const common::data::ByteBuffer& buf,
                               const arma::uword frame_nb) const
{
    if(buf.size() != chunk.n_cols * chunk.n_slices * sizeof(T1))
        common_die(logger_, -1, "invalid buffer size");

    auto it = buf.cbegin();
    arma::uword j = 0, k = 0;
    arma::uword n = 0;
    while(it != buf.cend())
    {
        auto x = reinterpret_cast<const T1*>(&*it);
        it += sizeof(*x);
        chunk.at(frame_nb, j, k) = T2(*x);
        n++;
        j = n % chunk.n_cols;
        k = n / chunk.n_cols;
    }
    return 0;
}

template<>
inline
int source<iq<int16_t>, arma::cx_double>::fill_frame(Chunk<arma::cx_double>& chunk,
                                                     const common::data::ByteBuffer& buf,
                                                     const arma::uword frame_nb) const
{
    if(buf.size() != chunk.n_cols * chunk.n_slices * sizeof(iq<int16_t>)) {
        common_die(logger_, -1, "invalid buffer size");
    }

    auto it = buf.cbegin();
    arma::uword j = 0, k = 0;
    arma::uword n = 0;
    while(it != buf.cend())
    {
        auto x = reinterpret_cast<const iq<int16_t>*>(&*it);
        it += sizeof(*x);
        chunk.at(frame_nb, j, k) = arma::cx_double(x->i, x->q);
        n++;
        j = n % chunk.n_cols;
        k = n / chunk.n_cols;
    }
    return 0;
}

template<>
inline
int source<iq<int16_t>, arma::cx_float>::fill_frame(Chunk<arma::cx_float>& chunk,
                                                    const common::data::ByteBuffer& buf,
                                                    const arma::uword frame_nb) const
{
    if(buf.size() != chunk.n_cols * chunk.n_slices * sizeof(iq<int16_t>)) {
        common_die(logger_, -1, "invalid buffer size");
    }

    auto it = buf.cbegin();
    arma::uword j = 0, k = 0;
    arma::uword n = 0;
    while(it != buf.cend())
    {
        auto x = reinterpret_cast<const iq<int16_t>*>(&*it);
        it += sizeof(*x);
        chunk.at(frame_nb, j, k) = arma::cx_float(x->i, x->q);
        n++;
        j = n % chunk.n_cols;
        k = n / chunk.n_cols;
    }
    return 0;
}


} /* namespace filter */

#endif /* SOURCE_FILTER_H */
