#include "common.h"
#include "electrical_value.h"

ElectricalValue combine_electrical_values(ElectricalValue v1, ElectricalValue v2) {
    switch (v1) {
    case EmptyElectricalValue:
        return v2;
    case HiElectricalValue:
        if (v2 == LoElectricalValue) {
            throw ElectricalValueException();
        }
        return HiElectricalValue;
    case LoElectricalValue:
        if (v2 == HiElectricalValue) {
            throw ElectricalValueException();
        }
        return LoElectricalValue;
    case PullLoElectricalValue:
        if (v2 == HiElectricalValue) {
            return HiElectricalValue;
        } else {
            return PullLoElectricalValue;
        }
    case PullHiElectricalValue:
        if (v2 == LoElectricalValue) {
            return LoElectricalValue;
        } else {
            return PullHiElectricalValue;
        }
    }
    ASSERT(false);
}

std::string electrical_value_to_str(ElectricalValue v) {
    switch (v) {
    case EmptyElectricalValue:
        return "Empty";
    case HiElectricalValue:
        return "Hi";
    case LoElectricalValue:
        return "Lo";
    case PullHiElectricalValue:
        return "PullHi";
    case PullLoElectricalValue:
        return "PullLo";
    }
    ASSERT(false);
}
