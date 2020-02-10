#include "common/data.h"
#include "source_filter.h"
#include "chunk.h"
#include "link.h"

namespace filter
{

Source::Source(common::Logger logger, common::data::Handler * data_handler, common::data::type type):
    Log(logger), Filter(logger), common::data::Consumer(logger, data_handler), type_(type)
{
}

Source::~Source()
{
}

int Source::set_format(uint16_t sample_size, uint16_t nb_samples, uint16_t nb_slots)
{
    sample_size_ = sample_size;
    nb_samples_  = nb_samples;
    nb_slots_    = nb_slots;

    return 0;
}

int Source::set_nb_frames(uint16_t nb_frames)
{
    nb_frames_ = nb_frames;
    return 0;
}

int Source::activate()
{
    int ret;
    std::vector<common::data::ByteBuffer> data;

    common::data::ByteBuffer buf
        = {1,  10, 2,  0, 3,  0, 4,  0, 5,  0, 6,  0, 7,  0, 8,  0, 9,  0, 10, 0,
           11, 10, 12, 0, 13, 0, 14, 0, 15, 0, 16, 0, 17, 0, 18, 0, 19, 0, 20, 0};

    //ret = pop_chunk(type_, nb_frames_, data);
    //common_die_zero(logger_, ret, -1, "source {} failed to pop chunk", this->name_);

    data.push_back(buf);
    data.push_back(buf);

    Chunk chunk(logger_, nb_frames_, nb_samples_, nb_slots_);

    uint16_t i = 0;
    for(auto& buf: data)
    {
        ret = chunk.fill_frame(buf, i);
        common_die_zero(logger_, ret, -2, "source {} failed to fill frame", this->name_);
        i++;
    }

    outputs_.at(0)->push(std::move(chunk));
    return 0;
}

} /* namespace filter */
