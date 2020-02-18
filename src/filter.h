#ifndef FILTER_H
#define FILTER_H

#include <string>
#include <vector>

#include "common/log.h"

class LinkInterface;
class Pipeline;

class Filter: virtual public common::Log
{
public:
    Filter(common::Logger logger): Log(logger) {}
    Filter(common::Logger logger, std::string name): Log(logger), name_(name) {}

    int add_input(LinkInterface& link)
    {
        inputs_.push_back(&link);
        return 0;
    }

    int add_output(LinkInterface& link)
    {
        outputs_.push_back(&link);
        return 0;
    }

    virtual int activate() = 0;

    bool is_ready() const {return ready_;};
    void set_ready()      {ready_ = true;};
    void reset_ready()    {ready_ = false;};

    void set_verbose()    {verbose_ = true;};

    std::string get_name() const {return name_;};

protected:
    std::string name_;

    Pipeline * pipeline_;
    std::vector<LinkInterface*> inputs_;
    std::vector<LinkInterface*> outputs_;

    bool ready_   = false;
    bool verbose_ = false;
};

#endif /* FILTER_H */
