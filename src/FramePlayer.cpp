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

#include "FramePlayer.h"
#include "utils.h"

#include <iostream>
#include <sstream>

#include <vrs/IndexRecord.h>

namespace rerun_vrs {
    struct FrameNumberDataLayout : public vrs::AutoDataLayout {
        vrs::DataPieceValue<uint64_t> frameNumber{"frame_number"};

        vrs::AutoDataLayoutEnd endLayout;
    };

    RerunFramePlayer::RerunFramePlayer(vrs::StreamId id, rerun::RecordingStream& rec)
        : id_{id}, rec_{rec}, entityPath_{add_quotes(id.getName())} {}

    bool RerunFramePlayer::onDataLayoutRead(
        const vrs::CurrentRecord& record, size_t blockIndex, vrs::DataLayout& layout
    ) {
        if (!enabled_)
            return false;

        std::ostringstream buffer;
        layout.printLayoutCompact(buffer);
        const auto& layout_str = buffer.str();

        rec_.set_time_seconds("timestamp", record.timestamp);

        if (record.recordType == vrs::Record::Type::CONFIGURATION) {
            // NOTE this is meta data from the sensor that doesn't change over time and only comes
            // in once in the beginning
            rec_.log_timeless(
                (entityPath_ + "/configuration").c_str(),
                rerun::TextDocument(layout_str)
            );
            return false;
        }

        if (record.recordType == vrs::Record::Type::DATA) {
            auto& config = getExpectedLayout<FrameNumberDataLayout>(layout, blockIndex);
            uint64_t frame_number;
            if (config.frameNumber.get(frame_number))
                rec_.set_time_sequence("frame_number", frame_number);

            // this is meta data per record and changes over time
            rec_.log((entityPath_ + "/data").c_str(), rerun::TextDocument(layout_str));
        }

        return true; // read next blocks, if any
    }

    bool RerunFramePlayer::onImageRead(
        const vrs::CurrentRecord& record, size_t /*blockIndex*/,
        const vrs::ContentBlock& contentBlock
    ) {
        std::shared_ptr<vrs::utils::PixelFrame> frame;
        bool frameValid = vrs::utils::PixelFrame::readFrame(frame, record.reader, contentBlock);

        if (frameValid) {
            // Log image to Rerun
            // NOTE currently we need to construct a vector to log an image, this will
            //  change in the future, see https://github.com/rerun-io/rerun/issues/3794
            rec_.log(
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
        return true; // read next blocks, if any
    }

    void RerunFramePlayer::convertFrame(std::shared_ptr<vrs::utils::PixelFrame>& frame) {
        /* if (blankMode_) { */
        /*     makeBlankFrame(frame); */
        /* } else { */
        /*     std::shared_ptr<vrs::utils::PixelFrame> convertedFrame = */
        /*         needsConvertedFrame_ ? getFrame(false) : nullptr; */
        /*     vrs::utils::PixelFrame::normalizeFrame(frame, convertedFrame, false); */
        /*     needsConvertedFrame_ = (frame != convertedFrame); // for next time! */
        /*     if (needsConvertedFrame_) { */
        /*         recycle(frame, true); */
        /*         frame = std::move(convertedFrame); */
        /*     } */
        /* } */
    }

    void RerunFramePlayer::makeBlankFrame(std::shared_ptr<vrs::utils::PixelFrame>& frame) {
        frame->init(vrs::PixelFormat::GREY8, frame->getWidth(), frame->getHeight());
        frame->blankFrame();
    }

    void RerunFramePlayer::recycle(
        std::shared_ptr<vrs::utils::PixelFrame>& frame, bool inputNotConvertedFrame
    ) {
        /* if (frame) { */
        /*     { */
        /*         std::vector<std::shared_ptr<vrs::utils::PixelFrame>>& frames = */
        /*             inputNotConvertedFrame ? inputFrames_ : convertedframes_; */
        /*         if (frames.size() < 10) { */
        /*             frames.emplace_back(std::move(frame)); */
        /*         } */
        /*     } */
        /*     frame.reset(); */
        /* } */
    }

    void RerunFramePlayer::imageJobsThreadActivity() {
        std::unique_ptr<ImageJob> job;
        while (imageJobs_.waitForJob(job)) {
            std::shared_ptr<vrs::utils::PixelFrame> frame = std::move(job->frame);
            // if we're behind, we just drop images except the last one!
            while (imageJobs_.getJob(job)) {
                recycle(frame, true);
                frame = std::move(job->frame);
            }
            bool frameValid = false;
            if (job->imageFormat == vrs::ImageFormat::RAW ||
                job->imageFormat == vrs::ImageFormat::VIDEO) {
                frameValid = (frame != nullptr);
            } else {
                if (!frame) {
                    frame = std::make_shared<vrs::utils::PixelFrame>();
                }
                frameValid = frame->readCompressedFrame(job->buffer, job->imageFormat);
            }
            if (frameValid) {
                convertFrame(frame);
                /* widget_->swapImage(frame); */
            }
            recycle(frame, !frameValid || !needsConvertedFrame_);
        }
    }

} // namespace rerun_vrs
