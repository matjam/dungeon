//
// Created by matjam on 12/20/2021.
//

#include "TilesetGenerator.h"

#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string.hpp>

using json = nlohmann::json;

void TilesetGenerator::run() {
    std::ifstream i("tileset_generator.json");
    json generatorConfigJson;
    i >> generatorConfigJson;

    std::ofstream o("data/tileset_manifest.json");
    json manifestJson;

    manifestJson["file"] = generatorConfigJson["file"];

    sf::Image sheetImage;
    sheetImage.create(
            generatorConfigJson["sheet_width"],
            generatorConfigJson["sheet_height"],
            sf::Color::Transparent
    );


    sf::Vector2u currentPosition{0, 0};
    std::string baseRemovePrefix = generatorConfigJson["remove_prefix"];

    unsigned int lastHeight = 0;

    for (auto &sheet: generatorConfigJson["sheets"]) {
        std::string name = sheet["name"];
        std::string path = sheet["path"];
        sf::Vector2u size{sheet["size"]["width"], sheet["size"]["height"]};
        std::string removePrefix = sheet["remove_prefix"];

        SPDLOG_INFO("processing {} path {} size {},{}", name, path, size.x, size.y);

        manifestJson["sets"][name]["size"] = {
                {"width",  size.x},
                {"height", size.y},
        };

        // put the new sheet on a new line in the final tilesheet if we're not already on a new line
        if (currentPosition.x != 0) {
            currentPosition.x = 0;
            currentPosition.y += lastHeight;
        }

        // read every image and write it to the sheet
        for (auto const &dir_entry: std::filesystem::directory_iterator{path}) {
            auto path = std::filesystem::path(dir_entry.path()).make_preferred();
            auto tileName = path.stem().string();
            boost::replace_first(tileName, baseRemovePrefix, "");
            boost::replace_first(tileName, removePrefix, "");
            SPDLOG_INFO("tileName: {} location: {},{}", tileName, currentPosition.x, currentPosition.y);

            // read the source image
            sf::Image sourceImage;
            if (!sourceImage.loadFromFile(path.string())) {
                SPDLOG_CRITICAL("couldn't open file at {}", path.string());
                return;
            }

            // write to the spritesheet
            sheetImage.copy(sourceImage, currentPosition.x, currentPosition.y, sf::IntRect{}, true);

            manifestJson["sets"][name]["tiles"][tileName] = {
                    {"x", currentPosition.x},
                    {"y", currentPosition.y},
            };

            currentPosition.x += size.x;
            // make sure the next tile can fit
            if (currentPosition.x + size.x > sheetImage.getSize().x) {
                currentPosition.y += size.y;
                currentPosition.x = 0;
            }
        }

        lastHeight = size.y;
    }

    if (!sheetImage.saveToFile(generatorConfigJson["file"])) {
        SPDLOG_CRITICAL("Could not write tilesheet to {}", generatorConfigJson["file"]);
    }

    o << std::setw(4) << manifestJson << std::endl;
    o.close();
}
