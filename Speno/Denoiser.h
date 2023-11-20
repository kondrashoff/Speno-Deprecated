#pragma once

#include <OpenImageDenoise/oidn.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

class Denoiser {
public:

    Denoiser() = default;

    Denoiser(int other_width, int other_height) {
        width = other_width;
        height = other_height;

        device.commit();

        color_buffer = device.newBuffer(width * height * 4 * sizeof(float));
        albedo_buffer = device.newBuffer(width * height * 4 * sizeof(float));
        normal_buffer = device.newBuffer(width * height * 4 * sizeof(float));
        output_buffer = device.newBuffer(width * height * 4 * sizeof(float));

        filter.setImage("color", color_buffer, oidn::Format::Float4, width, height, 0, 0, 0);
        filter.setImage("albedo", albedo_buffer, oidn::Format::Float4, width, height, 0, 0, 0);
        filter.setImage("normal", normal_buffer, oidn::Format::Float4, width, height, 0, 0, 0);
        filter.setImage("output", output_buffer, oidn::Format::Float4, width, height, 0, 0, 0);

        filter.set("quality", OIDN_QUALITY_HIGH);
        filter.set("srgb", false);
        filter.set("hdr", true);

        filter.commit();
    }

    void denoiseAndSaveImage() {
        filter.execute();

        const char* errorMessage;
        if (device.getError(errorMessage) != oidn::Error::None) {
            std::cout << "Error: " << errorMessage << std::endl;
            exit(EXIT_FAILURE);
        }

        float* denoised_data = (float*)output_buffer.getData();
        unsigned char* color_data = new unsigned char[width * height * 4];

        for (int i = 0; i < width * height * 4; i++) {
            color_data[i] = (unsigned char)(denoised_data[i] * 255.0);
        }

        std::time_t now = std::time(nullptr);
        char timestamp[20];
        std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", std::localtime(&now));
        std::string filename = "C:/GAMES/screenshot_" + std::string(timestamp) + ".png";

        stbi_flip_vertically_on_write(1);
        stbi_write_png(filename.c_str(), width, height, 4, color_data, 0);

        stbi_image_free(color_data);
    }

public:

    oidn::BufferRef color_buffer;
    oidn::BufferRef albedo_buffer;
    oidn::BufferRef normal_buffer;
    oidn::BufferRef output_buffer;

private:

    oidn::DeviceRef device = oidn::newDevice();
    oidn::FilterRef filter = device.newFilter("RT");

    int width, height;
};