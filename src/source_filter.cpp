#include "common/data.h"
#include "source_filter.h"
#include "chunk.h"
#include "link.h"
#include <memory>

namespace filter
{

Source::Source(common::Logger logger, common::data::Handler * data_handler, common::data::type type):
    Log(logger), Filter(logger), common::data::Consumer(logger, data_handler), type_(type)
{
    ready_ = true;
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
    log_debug(logger_, "source {} activated", this->name_);

    int ret;
    std::vector<common::data::ByteBuffer> data;

    ret = pop_chunk(type_, nb_frames_, data);
    common_die_zero(logger_, ret, -1, "source {} failed to pop chunk", this->name_);

    auto chunk = std::make_shared<Chunk>(logger_, nb_frames_, nb_samples_, nb_slots_);

    uint16_t i = 0;
    for(auto& buf: data)
    {
        ret = chunk->fill_frame(buf, i);
        common_die_zero(logger_, ret, -2, "source {} failed to fill frame", this->name_);
        i++;
    }

    if (verbose_)
        chunk->print();

    outputs_.at(0)->push(chunk);
    return 0;
}

} /* namespace filter */
