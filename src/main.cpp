#include <iostream>
#include <set>
#include <vector>

#include <vrs/RecordFileReader.h>
#include <vrs/RecordFormatStreamPlayer.h>
#include <vrs/StreamPlayer.h>
#include <rerun.hpp>

#include "frame_player.hpp"
#include "imu_player.hpp"

int main(int argc, const char* argv[]) {
    std::cout << "Rerun SDK Version:" << rerun::version_string() << std::endl;

    auto rec = std::make_shared<const rerun::RecordingStream>("rerun_example_vrs");
    rec->spawn().exit_on_failure();

    if (argc != 2) {
        std::cout << "Usage: ./log_vrs path/to/vrsfile.vrs" << std::endl;
        return 0;
    }

    std::string vrs_path(argv[1]);

    vrs::RecordFileReader reader;
    if (reader.openFile(vrs_path) == 0) {
        // for each type of stream there is one player that logs the read data to
        // Rerun
        std::vector<std::unique_ptr<vrs::StreamPlayer>> stream_players;

        const std::set<vrs::StreamId>& streamIds = reader.getStreams();
        for (auto id : streamIds) {
            std::cout << id.getName() << " (" << id.getTypeName() << ")"
                      << ": ";
            if (reader.mightContainImages(id)) {
                std::cout << "Handled by FramePlayer" << std::endl;
                stream_players.emplace_back(std::make_unique<rerun_vrs::FramePlayer>(id, rec));
                reader.setStreamPlayer(id, stream_players.back().get());
            } else if (rerun_vrs::might_contain_imu_data(id)) {
                std::cout << "Handled by IMUPlayer" << std::endl;
                stream_players.emplace_back(std::make_unique<rerun_vrs::IMUPlayer>(id, rec));
                reader.setStreamPlayer(id, stream_players.back().get());
            } else {
                std::cout << "No player available. Skipped." << std::endl;
            }
        }
        reader.readAllRecords();
    }

    return 0;
}
