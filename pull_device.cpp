#include "common.h"
#include "device.h"

Rgb PullDevice::yellow = Rgb(0xff, 0xf2, 0);

Device *PullDevice::create(void) {
    return new PullDevice();
}

std::string PullDevice::name(void) {
    return "PullDevice";
}

bool PullDevice::parse(AspngSurface *surface, Coord coord) {
    // Let's look for a source pixel ..
    this->patch_source_or_sink = this->flood(surface, coord, SourceDevice::color);
    this->pull_type = PullHi;
    if (this->patch_source_or_sink.size() == 0) {
        // .. if no source, let's look for a sink.
        this->patch_source_or_sink = this->flood(surface, coord, SinkDevice::color);
        this->pull_type = PullLo;
    }
    // If we found a source or a sink pixel, let's look for the yellow pixel
    // next to it.
    if (this->patch_source_or_sink.size() == 1) {
        int offs[4][2] = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } };
        for (int i = 0; i < 4; i++) {
            this->patch_yellow = this->flood(surface, Coord(coord.x + offs[i][0], coord.y + offs[i][1]), PullDevice::yellow);
            if (this->patch_yellow.size() == 1) {
                return true;
            }
        }
    }
    return false;
}

std::list<Patch *> PullDevice::all_patches(void) {
    std::list<Patch *> all_patches;
    all_patches.push_back(&(this->patch_source_or_sink));
    all_patches.push_back(&(this->patch_yellow));
    return all_patches;
}

std::tuple<LinkResult, PortType, std::string> PullDevice::prelink(Patch *patch, std::shared_ptr<Device> d) {
    if (patch == &(this->patch_yellow)) {
        // The yellow bit can only touch copper and the background.
        if (std::dynamic_pointer_cast<CopperDevice>(d))
            return std::make_tuple(CanLink, NoSpecialMeaning, "");
        if (std::dynamic_pointer_cast<BackgroundDevice>(d))
            return std::make_tuple(CanTouch, NoSpecialMeaning, "");
    } else if (patch == &(this->patch_source_or_sink)) {
        // The source/sink can only touch background.
        if (std::dynamic_pointer_cast<BackgroundDevice>(d))
            return std::make_tuple(CanTouch, NoSpecialMeaning, "");
    }
    return std::make_tuple(LinkError, NoSpecialMeaning, "can't touch that");
}

bool PullDevice::link(void) {
    return true;
}

std::list<std::shared_ptr<Port>> PullDevice::propagate(std::shared_ptr<Port> port) {
    std::list<std::shared_ptr<Port>> next_ports = this->all_ports();
    next_ports.remove(port);
    return next_ports;
}

ElectricalValue PullDevice::get_value_at_port(std::shared_ptr<Port>) {
    switch (this->pull_type) {
    case PullHi:
        return PullHiElectricalValue;
    case PullLo:
        return PullLoElectricalValue;
    }
    ASSERT(0);
}

void PullDevice::apply_new_value(std::shared_ptr<Port>, ElectricalValue) {
    // Deliberately empty.
}

Rgb PullDevice::get_draw_color(Patch *patch) {
    if (patch == &(this->patch_yellow)) {
        return PullDevice::yellow;
    } else if (patch == &(this->patch_source_or_sink)) {
        switch (this->pull_type) {
        case PullHi:
            return SourceDevice::color;
        case PullLo:
            return SinkDevice::color;
        }
    }
    ASSERT(0);
}
