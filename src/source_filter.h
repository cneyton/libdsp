#ifndef SOURCE_FILTER_H
#define SOURCE_FILTER_H

#include "common/log.h"
#include "common/data.h"
#include "filter.h"
#include "chunk.h"
#include "link.h"

namespace filter
{

template<typename T>
class source: public Filter, public common::data::Consumer
{
public:
    source(common::Logger logger, common::data::Handler * data_handler, common::data::type type):
        Log(logger), Filter(logger), common::data::Consumer(logger, data_handler), type_(type) {}
    ~source() {}

    int set_chunk_size(uint16_t nb_frames, uint16_t nb_samples, uint16_t nb_slots)
    {
        nb_frames_ = nb_frames;
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

        auto chunk = std::make_shared<Chunk<T>>(logger_, chunk_size);

        uint16_t i = 0;
        for(auto& buf: data)
        {
            ret = chunk->fill_frame(buf, i);
            common_die_zero(logger_, ret, -2, "source {} failed to fill frame", this->name_);
            i++;
        }

        if (verbose_)
            chunk->print();

        Link<T> * output = dynamic_cast<Link<T>*>(outputs_.at(0));
        output->push(chunk);
        return 0;
    }


private:
    common::data::type type_;
    uint16_t nb_frames_;
    uint16_t nb_samples_;
    uint16_t nb_slots_;
};

} /* namespace filter */

#endif /* SOURCE_FILTER_H */
