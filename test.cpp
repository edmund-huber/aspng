#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <queue>
#include <memory>
#include <set>

#include "aspng.h"
#include "common.h"
#include "device.h"
#include "png.h"

std::string test(std::string path, std::string test_name) {
    auto png = Png::read(path + "/_.png");
    if (png == nullptr)
        return "missing _.png";

    std::string error;
    auto aspng = Aspng(png, error);
    if (error != "") {
        return error;
    }

    std::list<int> output_wrong_at;
    for (int frame_counter = 0; ; frame_counter++) {
        auto png = Png::read(path + "/" + std::to_string(frame_counter) + ".png");

        error = aspng.step();
        if (error != "") {
            return error;
        }

        // If there's no PNG to compare the output against, then the test is
        // done.
        if (png == nullptr)
            break;

        // Store the output image.
        Png out_png(png->get_width(), png->get_height());
        aspng.draw(&out_png);
        std::filesystem::create_directory("tests_output");
        std::filesystem::create_directory("tests_output/" + test_name);
        out_png.write("tests_output/" + test_name + "/" + std::to_string(frame_counter) + ".png");

        // Compare the output image to the expected image.
        ASSERT(out_png.get_width() == png->get_width());
        ASSERT(out_png.get_height() == png->get_height());
        bool wrong = false;
        for (int32_t x = 0; x < out_png.get_width(); x++) {
            for (int32_t y = 0; y < out_png.get_height(); y++) {
                if (out_png.get_pixel(x, y) != png->get_pixel(x, y)) {
                    wrong = true;
                    break;
                }
            }
        }
        if (wrong) {
            output_wrong_at.push_back(frame_counter);
        }

        // TODO: flip the clock value
    }

    if (!output_wrong_at.empty()) {
        std::string fail_reason = "output wrong at";
        for (auto i = output_wrong_at.begin(); i != output_wrong_at.end(); i++) {
            fail_reason += " " + std::to_string(*i);
        }
        return fail_reason;
    }

    return "";
}

int main(void) {
    // Run all tests under tests/.
    int pass = 0;
    int fail = 0;
    for (auto &entry : std::filesystem::directory_iterator("tests/")) {
        auto test_name = *(--entry.path().end());
        std::cout << test_name << " .. ";
        auto fail_reason = test(entry.path(), test_name);
        std::string expect_fail_reason = "";
        auto expect_fail_path = entry.path().string() + "/expect_fail";
        if (std::filesystem::exists(expect_fail_path)) {
            std::ifstream f(expect_fail_path);
            std::stringstream buffer;
            buffer << f.rdbuf();
            expect_fail_reason = buffer.str();
            if (expect_fail_reason != "") {
                expect_fail_reason.pop_back();
            }
        }
        std::string status;
        if ((fail_reason == "") && (expect_fail_reason == "")) {
            std::cout << "PASS" << std::endl;
            pass++;
        } else if (fail_reason == expect_fail_reason) {
            std::cout << "PAIL" << std::endl;
            pass++;
        } else {
            std::cout << "FAIL - got \"" << fail_reason << "\", expected: \"" << expect_fail_reason << "\"" << std::endl;
            fail++;
        }
    }

    std::cout << "pass: " << pass << ", fail: " << fail << std::endl;

    if (fail == 0) {
        return 0;
    } else {
        return 1;
    }
}
