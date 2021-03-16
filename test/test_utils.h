#include <cnpy.h>

#include "common/log.h"

#include "dsp/pipeline.h"
#include "dsp/source_filter.h"
#include "dsp/sink_filter.h"

using namespace dsp;

template<typename T>
class NpySource: public Filter
{
public:
    NpySource(common::Logger logger, std::string filename):
        Filter(logger, "npy source"),
        filename_(filename)
    {
        Pad p {.name="out", .format=Format()};
        output_pads_.insert({p.name, p});

        cnpy::NpyArray np_array = cnpy::npy_load(filename_);
        arma::uword n_rows   = np_array.shape.at(2);
        arma::uword n_cols   = np_array.shape.at(1);
        arma::uword n_slices = np_array.shape.at(0);

        data_ = arma::Cube<T>(np_array.data<T>(), n_rows, n_cols, n_slices);
    }

    int activate() override
    {
        log_debug(logger_, "{} filter activated, i = {}", this->name_, i_);
        auto output = dynamic_cast<Link<T>*>(outputs_.at("out"));
        auto fmt    = output->format();

        arma::uword row_beg = i_ * fmt.n_rows;
        arma::uword row_end = (i_+1) * fmt.n_rows;
        if (row_end <= data_.n_rows) {
            auto chunk = std::make_shared<Chunk<T>>(data_.rows(row_beg, row_end - 1));
            output->push(chunk);
            i_++;
            return 1;
        } else {
            log_debug(logger_, "eof");
            output->eof_reached();
            return 0;
        }
    }

    void reset() override
    {
        i_ = 0;
    }

    Contract negotiate_format() override
    {
        return Contract::supported_format;
    }

    arma::SizeCube get_fmt()
    {
        return arma::size(data_);
    }

private:
    std::string    filename_;
    arma::Cube<T>  data_;
    arma::uword    i_ = 0;
};


template<typename T>
class NpySink: public Filter
{
public:
    NpySink(common::Logger logger, arma::SizeCube fmt, std::string_view name = "sink"):
        Filter(logger, name), data_(arma::Cube<T>(fmt))
    {
        Pad p {.name="in", .format=Format()};
        input_pads_.insert({p.name, p});
    }

    int activate() override
    {
        log_debug(logger_, "{} filter activated, i = {}", name_, i_);

        auto input = dynamic_cast<Link<T>*>(inputs_.at("in"));

        int ret;
        auto chunk = std::make_shared<Chunk<T>>();
        ret = input->pop(chunk);
        common_die_zero(this->logger_, ret, -1, "failed to pop chunk");
        if (!ret) return 0;

        data_.rows(i_ * chunk->n_rows, (i_+1) * chunk->n_rows - 1) = *chunk;
        i_++;

        return 1;
    }

    void reset() override
    {
        i_ = 0;
    }

    Contract negotiate_format() override
    {
        return Contract::supported_format;
    }

    void dump(std::string filename)
    {
        auto input = dynamic_cast<Link<T>*>(inputs_.at("in"));
        while (!input->eof()) {
            activate();
            this->pipeline_->run();
        }
        auto fmt   = input->format();
        data_.resize(fmt.n_rows * i_, fmt.n_cols, fmt.n_slices);
        cnpy::npy_save(filename, data_.memptr(), {data_.n_slices, data_.n_cols, data_.n_rows}, "w");
    }

private:
    arma::Cube<T>  data_;
    arma::uword    i_ = 0;
};
