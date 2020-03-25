#include <thread>
#include <cnpy.h>

#include "common/log.h"
#include "common/data.h"

#include "pipeline.h"
#include "source_filter.h"
#include "sink_filter.h"

class Handler: public common::data::Handler
{
public:
    Handler(common::Logger logger, Pipeline * pipeline):
        common::data::Handler(logger), pipeline_(pipeline) {}
    virtual ~Handler() {}

    virtual int data_pushed()
    {
        pipeline_->wakeup();
        return 0;
    }

    virtual int eof()
    {
        pipeline_->eof();
        return 0;
    }

private:
    Pipeline * pipeline_;
};

template<typename T>
class Producer: public common::data::Producer
{
public:
    Producer(common::Logger logger, common::data::Handler * h, std::string filename,
             arma::uword period=1, arma::uword queue_size=1000):
        Log(logger), common::data::Producer(logger, h), filename_(filename),
        period_(period), queue_size_(queue_size)
    {
        cnpy::NpyArray np_array = cnpy::npy_load(filename_);
        arma::uword n_rows   = np_array.shape.at(2);
        arma::uword n_cols   = np_array.shape.at(1);
        arma::uword n_slices = np_array.shape.at(0);

        data_ = arma::Cube<T>(np_array.data<T>(), n_rows, n_cols, n_slices);
    }

    virtual ~Producer() {}

    void run()
    {
        size_t row_size = data_.n_cols * data_.n_slices * sizeof(T);
        get_handler()->reinit_queue(common::data::type::us, row_size, queue_size_);
        for (arma::uword i = 0; i < data_.n_rows; ++i) {
            arma::Col<T> row = arma::vectorise(data_.row(i));
            uint8_t * memptr = reinterpret_cast<uint8_t*>(row.memptr());
            common::data::ByteBuffer buf(memptr, memptr + row_size);
            push(common::data::type::us, buf);
            std::this_thread::sleep_for(std::chrono::milliseconds(period_));
        }
        eof();
    }

    arma::SizeCube get_fmt()
    {
        return arma::size(data_);
    }

private:
    std::string   filename_;
    arma::Cube<T> data_;
    arma::uword   period_;
    arma::uword   queue_size_;
};

template<typename T>
class NpySink: public filter::sink<T>
{
public:
    NpySink(common::Logger logger, arma::SizeCube fmt):
        common::Log(logger), filter::sink<T>(logger), data_(arma::Cube<T>(fmt)) {}

    virtual ~NpySink() {}

    virtual int activate()
    {
        auto input = dynamic_cast<Link<T>*>(this->inputs_.at(0));
        if (input->empty()) return 0;

        auto chunk = input->front();
        input->pop();

        data_.rows(i_ * chunk->n_rows, (i_+1) * chunk->n_rows - 1) = *chunk;
        i_++;

        return 1;
    }

    void dump(std::string filename)
    {
        cnpy::npy_save(filename, data_.memptr(), {data_.n_slices, data_.n_cols, data_.n_rows}, "w");
    }

private:
    arma::Cube<T>  data_;
    arma::uword    i_ = 0;
};
