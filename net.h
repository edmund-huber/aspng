#ifndef __NET_H__
#define __NET_H__

#include <memory>
#include <set>

#include "bounding_box.h"
#include "port.h"

class Net {
public:
    Net(std::shared_ptr<Port>, std::set<std::shared_ptr<Port>> &);
    void compute_new_value(void);
    void apply_new_value(void);

private:
    Net();
    BoundingBox get_bounding_box(void);

    std::set<std::shared_ptr<Port>> ports_in_net;
    ElectricalValue new_value;
};

#endif
