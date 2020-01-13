#ifndef LINK_H
#define LINK_H

#include <memory>

class Filter;

class Link
{
public:
private:
    std::shared_ptr<Filter> src;
    std::shared_ptr<Filter> dst;
};

#endif /* LINK_H */
