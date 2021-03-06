#include "common.h"
#include "device.h"
#include "simple_aspng_surface.h"

Rgb BaseTemplateDevice::color = Rgb(0x11, 0x9e, 0);

Png *font = nullptr;
std::vector<SimpleAspngSurface> glyphs;

bool BaseTemplateDevice::parse(AspngSurface *surface, Coord coord) {
    // Do the one-time font loading if it hasn't been done yet ..
    if (font == nullptr) {
        font = Png::read("font.png");
        ASSERT(font != nullptr);
        ASSERT(font->get_height() == 5);

        // .. and chop the font up into glyphs.
        int32_t glyph_begin_x = 0;
        SimpleAspngSurface glyph(1, font->get_height());
        for (int32_t font_x = 0; font_x < font->get_width(); font_x++) {
            bool blank_column = true;
            for (int32_t font_y = 0; font_y < font->get_height(); font_y++) {
                Rgb pixel = font->get_pixel(font_x, font_y);
                if (pixel == Rgb(0, 0, 0)) {
                    blank_column = false;
                } else {
                    ASSERT(pixel == BaseTemplateDevice::color);
                }
                glyph.resize(font_x - glyph_begin_x + 1, font->get_height());
                glyph.set_pixel(font_x - glyph_begin_x, font_y, pixel);
            }

            if (blank_column && (font_x > 0) && (glyph.get_width() > 1)) {
                glyph_begin_x = font_x;
                glyphs.push_back(glyph);
                glyph.resize(1, font->get_height());
            }
        }
        ASSERT(glyphs.size() == 26);
    }

    // Find the extents of what we're dealing with.
    Patch p = this->flood(surface, coord, BaseTemplateDevice::color);
    if (p.size() == 0) {
        return false;
    }
    int32_t min_x, max_x, min_y, max_y;
    p.get_bounding_box(min_x, max_x, min_y, max_y);

    // There should be a tab at the top left with the name of the device.
    int32_t surface_x = min_x;
    while (true) {
        int glyphs_found = 0;
        int which = 0;
        for (auto i = glyphs.begin(); i != glyphs.end(); i++, which++) {
            auto glyph = *i;
            if (surface->contains(&glyph, surface_x, min_y)) {
                // Add the character and add to the patch.
                this->tab += (char)('a' + which);
                glyphs_found++;
                for (int32_t patch_x = surface_x; patch_x < surface_x + glyph.get_width(); patch_x++) {
                    for (int32_t patch_y = min_y; patch_y < min_y + glyph.get_height(); patch_y++) {
                        this->patch.insert(Coord(patch_x, patch_y));
                    }
                }
                surface_x += glyph.get_width() - 1;
            }
        }
        if (glyphs_found == 0) {
            break;
        }
    }

    // Instantiate the underlying device named in the tab.
    if (tab.find(this->prefix()) != 0) {
        return false;
    }
    std::string param = tab.substr(this->prefix().length());
    if (!this->sub_parse(surface, min_x + 1, min_y + 5, max_x - 1, max_y - 1, param)) {
        return false;
    }

    // Add the box to the patch. (If it's not there, that's a parse error.)
    // (Top bit.)
    for (int32_t x = min_x; x <= max_x; x++) {
        if (surface->get_pixel(x, min_y + 4) != BaseTemplateDevice::color) {
            return false;
        }
        this->patch.insert(Coord(x, min_y + 4));
    }
    // (Right bit.)
    for (int32_t y = min_y + 4; y <= max_y; y++) {
        if (surface->get_pixel(max_x, y) != BaseTemplateDevice::color) {
            return false;
        }
        this->patch.insert(Coord(max_x, y));
    }
    // (Left bit.)
    for (int32_t y = min_y + 4; y <= max_y; y++) {
        if (surface->get_pixel(min_x, y) != BaseTemplateDevice::color) {
            return false;
        }
        this->patch.insert(Coord(min_x, y));
    }
    // (Bottom bit.)
    for (int32_t x = min_x; x <= max_x; x++) {
        if (surface->get_pixel(x, max_y) != BaseTemplateDevice::color) {
            return false;
        }
        this->patch.insert(Coord(x, max_y));
    }

    return true;
}

std::list<Patch *> BaseTemplateDevice::all_patches(void) {
    std::list<Patch *> all_patches;
    all_patches.push_back(&(this->patch));
    auto sub_patches = this->sub_patches();
    for (auto i = sub_patches.begin(); i != sub_patches.end(); i++) {
        all_patches.push_back(*i);
    }
    return all_patches;
}

std::tuple<LinkResult, PortType, std::string> BaseTemplateDevice::prelink(Patch *, std::shared_ptr<Device> d) {
    if (std::dynamic_pointer_cast<CopperDevice>(d))
        return std::make_tuple(CanLink, NoSpecialMeaning, "");
    if (std::dynamic_pointer_cast<BackgroundDevice>(d))
        return std::make_tuple(CanTouch, NoSpecialMeaning, "");
    return std::make_tuple(LinkError, NoSpecialMeaning, "must touch copper or background");
}

void BaseTemplateDevice::draw(AspngSurface *surface) {
    // Draw the frame.
    std::list<Patch *> just_this_patch;
    just_this_patch.push_back(&(this->patch));
    this->draw_helper(surface, just_this_patch);

    // Draw in the tab letters.
    int32_t min_x, max_x, min_y, max_y;
    this->patch.get_bounding_box(min_x, max_x, min_y, max_y);
    int32_t glyph_x = min_x;
    for (auto i = this->tab.begin(); i != this->tab.end(); i++) {
        auto c = *i;
        auto glyph = &glyphs[c - 'a'];
        surface->copy(glyph, glyph_x, min_y);
        glyph_x += glyph->get_width() - 1;
    }

    // Make the subdevice draw.
    this->sub_draw(surface, min_x + 1, min_y + 5, max_x - 1, max_y - 1);
}

Rgb BaseTemplateDevice::get_draw_color(Patch *) {
    return BaseTemplateDevice::color;
}
