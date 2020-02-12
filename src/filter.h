#ifndef FILTER_H
#define FILTER_H

#include <string>
#include <vector>

#include "common/log.h"

class Link;
class Pipeline;

class Filter: virtual public common::Log
{
public:
    Filter(common::Logger logger);
    Filter(common::Logger logger, std::string name);

    int add_input(Link& link);
    int add_output(Link& link);

    virtual int activate() = 0;

    bool is_ready() const {return ready_;};
    void set_ready()      {ready_ = true;};
    void reset_ready()    {ready_ = false;};

    std::string get_name() const {return name_;};

protected:
    std::string name_;

    Pipeline * pipeline_;
    std::vector<Link*> inputs_;
    std::vector<Link*> outputs_;

    bool ready_ = false;
};

#endif /* FILTER_H */
