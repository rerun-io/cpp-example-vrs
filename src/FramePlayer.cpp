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

#include "FramePlayer.h"
#include "utils.h"

namespace rerun_vrs {

    struct FrameNumberDataLayout : public vrs::AutoDataLayout {
        vrs::DataPieceValue<uint64_t> frameNumber{"frame_number"};

        vrs::AutoDataLayoutEnd endLayout;
    };

    FramePlayer::FramePlayer(vrs::StreamId id, std::shared_ptr<const rerun::RecordingStream> rec)
        : id_{id}, rec_{rec}, entityPath_{add_quotes(id.getName())} {}

    bool FramePlayer::onDataLayoutRead(
        const vrs::CurrentRecord& record, size_t blockIndex, vrs::DataLayout& layout
    ) {
        if (!enabled_)
            return false;

        std::ostringstream buffer;
        layout.printLayoutCompact(buffer);
        const auto& layout_str = buffer.str();

        rec_->set_time_seconds("timestamp", record.timestamp);

        if (record.recordType == vrs::Record::Type::CONFIGURATION) {
            // NOTE this is meta data from the sensor that doesn't change over time and only comes
            // in once in the beginning
            rec_->log_timeless(
                (entityPath_ + "/configuration").c_str(),
                rerun::TextDocument(layout_str)
            );
        }

        if (record.recordType == vrs::Record::Type::DATA) {
            auto& config = getExpectedLayout<FrameNumberDataLayout>(layout, blockIndex);
            uint64_t frame_number;
            if (config.frameNumber.get(frame_number))
                rec_->set_time_sequence("frame_number", frame_number);

            // this is meta data per record and changes over time
            rec_->log((entityPath_ + "/data").c_str(), rerun::TextDocument(layout_str));
        }

        return true;
    }

    bool FramePlayer::onImageRead(
        const vrs::CurrentRecord& record, size_t /*blockIndex*/,
        const vrs::ContentBlock& contentBlock
    ) {
        std::shared_ptr<vrs::utils::PixelFrame> frame;
        bool frameValid = vrs::utils::PixelFrame::readFrame(frame, record.reader, contentBlock);

        if (frameValid) {
            // Log image to Rerun
            // NOTE Rerun assumes row major ordering for Images (i.e., TensorData) without any stride.
            //   Right now we don't check this properly, and just assume that there is no extra padding
            //   per pixel and / or per row.
            rec_->log(
                add_quotes(id_.getName()).c_str(),
                rerun::Image(
                    {frame->getHeight(),
                     frame->getWidth(),
                     frame->getSpec().getChannelCountPerPixel()},
                    std::move(frame->getBuffer())
                )
            );
        } else {
            std::cout << "Failed reading image with format \""
                      << contentBlock.image().getImageFormatAsString() << "\". Disabling player."
                      << std::endl;
            enabled_ = false;
        }
        return true;
    }

} // namespace rerun_vrs
