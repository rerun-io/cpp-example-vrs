/* Modified from https://github.com/facebookresearch/vrs
 *
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <iostream>
#include <memory>
#include <sstream>

#include <vrs/DataLayout.h>
#include <vrs/utils/PixelFrame.h>

#include "frame_player.hpp"

namespace rerun_vrs {

    struct FrameNumberDataLayout : public vrs::AutoDataLayout {
        vrs::DataPieceValue<uint64_t> frameNumber{"frame_number"};

        vrs::AutoDataLayoutEnd endLayout;
    };

    FramePlayer::FramePlayer(vrs::StreamId id, std::shared_ptr<const rerun::RecordingStream> rec)
        : _id{id}, _rec{rec}, _entity_path{rerun::new_entity_path({id.getName()})} {}

    bool FramePlayer::onDataLayoutRead(
        const vrs::CurrentRecord& record, size_t block_index, vrs::DataLayout& layout
    ) {
        if (!_enabled) {
            return false;
        }

        std::ostringstream buffer;
        layout.printLayoutCompact(buffer);
        const auto& layout_str = buffer.str();

        _rec->set_time_seconds("timestamp", record.timestamp);

        if (record.recordType == vrs::Record::Type::CONFIGURATION) {
            // NOTE this is meta data from the sensor that doesn't change over time and only comes
            // in once in the beginning
            _rec->log_static(_entity_path + "/configuration", rerun::TextDocument(layout_str));
        }

        if (record.recordType == vrs::Record::Type::DATA) {
            auto& config = getExpectedLayout<FrameNumberDataLayout>(layout, block_index);
            uint64_t frame_number;
            if (config.frameNumber.get(frame_number)) {
                _rec->set_time_sequence("frame_number", frame_number);
            }

            // this is meta data per record and changes over time
            _rec->log(_entity_path + "/data", rerun::TextDocument(layout_str));
        }

        return true;
    }

    bool FramePlayer::onImageRead(
        const vrs::CurrentRecord& record, size_t /*block_index*/,
        const vrs::ContentBlock& content_block
    ) {
        std::shared_ptr<vrs::utils::PixelFrame> frame;
        bool frame_valid = vrs::utils::PixelFrame::readFrame(frame, record.reader, content_block);

        if (frame_valid) {
            // Log image to Rerun
            // NOTE Rerun assumes row major ordering for Images (i.e., TensorData) without any stride.
            //   Right now we don't check this properly, and just assume that there is no extra padding
            //   per pixel and / or per row.
            _rec->log(
                _entity_path,
                rerun::Image(
                    {frame->getHeight(),
                     frame->getWidth(),
                     frame->getSpec().getChannelCountPerPixel()},
                    frame->getBuffer()
                )
            );
        } else {
            std::cout << "Failed reading image with format \""
                      << content_block.image().getImageFormatAsString() << "\". Disabling player."
                      << std::endl;
            _enabled = false;
        }
        return true;
    }

} // namespace rerun_vrs
