#ifndef __ELECTRICAL_VALUE_H__
#define __ELECTRICAL_VALUE_H__

#include <string>

enum ElectricalValue {
    EmptyElectricalValue,
    HiElectricalValue,
    LoElectricalValue,
    PullHiElectricalValue,
    PullLoElectricalValue
};

ElectricalValue combine_electrical_values(ElectricalValue, ElectricalValue);
std::string electrical_value_to_str(ElectricalValue);

class ElectricalValueException : public std::exception {
    virtual const char *what() const throw() {
        return "ElectricalValueException";
    }
};

#endif
