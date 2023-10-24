#include <vrs/RecordFileReader.h>
#include <vrs/RecordFormatStreamPlayer.h>
#include <vrs/StreamPlayer.h>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
#include <iostream>
#include <rerun.hpp>
#include <set>
#include <vector>

#include "FramePlayer.h"

int main(int argc, const char* argv[]) {
    auto rec = rerun::RecordingStream("rerun_example_vrs");
    rec.connect().throw_on_failure();

    if (argc != 2) {
        std::cout << "Usage: ./log_vrs path/to/vrsfile.vrs" << std::endl;
        return 0;
    }

    std::string vrsPath(argv[1]);

    vrs::RecordFileReader reader;
    if (reader.openFile(vrsPath) == 0) {
        // for each type of stream there is one player that logs the read data to Rerun
        std::vector<std::unique_ptr<vrs::StreamPlayer>> streamPlayers;

        const std::set<vrs::StreamId>& streamIds = reader.getStreams();
        for (auto id : streamIds) {
            std::cout << id.getName() << " (" << id.getTypeName() << ")"
                      << ": ";
            if (reader.mightContainImages(id)) {
                std::cout << "Handled by FramePlayer" << std::endl;
                streamPlayers.emplace_back(std::make_unique<rerun_vrs::RerunFramePlayer>(id, rec));
                reader.setStreamPlayer(id, streamPlayers.back().get());
            }
            /* else if (id.getTypeId() == vrs::RecordableTypeId::ImuRecordableClass) { */
            /*   /1* streamPlayers.emplace_back(new ImagePlayer()); *1/ */
            /*   reader.setStreamPlayer(id, streamPlayers.back().get()); */
            /* } */
            else {
                std::cout << "No player available. Skipped." << std::endl;
            }
        }
        reader.readAllRecords();
    }

    return 0;
}
