#ifndef FILTER_H
#define FILTER_H

#include <string>

class Filter
{
public:
    virtual int activate() = 0;
protected:
    std::string name_;
    uint32_t nb_inputs;
    uint16_t nb_outputs;
    uint16_t samplecount;
private:
};

#endif /* FILTER_H */
