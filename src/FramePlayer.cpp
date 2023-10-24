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
    RerunFramePlayer::RerunFramePlayer(vrs::StreamId id, rerun::RecordingStream& rec)
        : id_{id}, rec_{rec} {}

    bool RerunFramePlayer::onDataLayoutRead(
        const vrs::CurrentRecord& record, size_t blockIndex, vrs::DataLayout& layout
    ) {
        if (!enabled_)
            return false;

        rec_.set_time_seconds("timestamp", record.timestamp);

        /* std::cout << "onDataLayoutRead " << std::endl; */
        std::ostringstream buffer;
        layout.printLayoutCompact(buffer);
        /* std::cout << buffer.str() << std::endl; */
        // TODO write this information to a markdown file

        // TODO figure out image width and height ?

        /* descriptions_.setDescription(record.recordType, blockIndex, text); */
        /* if (firstImage_ && record.recordType == Record::Type::CONFIGURATION) { */
        /*   vrs::DataPieceString* deviceType =
   * layout.findDataPieceString("device_type"); */
        /*   if (deviceType != nullptr) { */
        /*     widget_->setDeviceType(deviceType->get()); */
        /*   } */
        /* } */
        return true; // read next blocks, if any
    }

    bool RerunFramePlayer::onImageRead(
        const vrs::CurrentRecord& record, size_t /*blockIndex*/, const vrs::ContentBlock& contentBlock
    ) {
        /* std::cout << "onImageRead" << std::endl; */
        const auto& spec = contentBlock.image();
        std::shared_ptr<vrs::utils::PixelFrame> frame;
        const auto& imageFormat = spec.getImageFormat();
        const auto& imageFormatStr = spec.getImageFormatAsString();
        bool frameValid = false;
        /* std::cout << spec.getWidth() << "x" << spec.getHeight() << " " */
        /*           << spec.getPixelFormatAsString() << " " << spec.getImageFormatAsString() */
        /*           << std::endl; */

        if (imageFormat == vrs::ImageFormat::RAW) {
            frameValid = vrs::utils::PixelFrame::readRawFrame(frame, record.reader, spec);
            if (frameValid) {
                // Log image to Rerun
                // NOTE currently we need to construct a vector to log an image, this will
                //  change in the future, see https://github.com/rerun-io/rerun/issues/3794

                rec_.log(
                    add_quotes(id_.getName()).c_str(),
                    rerun::Image(
                        {spec.getHeight(), spec.getWidth(), spec.getChannelCountPerPixel()},
                        std::move(frame->getBuffer())
                    )
                );
            } else {
                std::cout << "Failed reading raw frame." << std::endl;
            }
            /* if (!firstImage_ && frameValid) { */
            /*   job = make_unique<ImageJob>(vrs::ImageFormat::RAW); */
            /* } */
        } else {
            std::cout << "Image format \"" << imageFormatStr
                      << "\" not supported. Disabling player." << std::endl;
            enabled_ = false;
        }
        return true; // read next blocks, if any

        /* } else if (imageFormat_ == vrs::ImageFormat::VIDEO) { */
        /*   // Video codec decompression happens here, but pixel format conversion is
   * asynchronous */
        /*   PixelFrame::init(frame, contentBlock.image()); */
        /*   frameValid = (tryToDecodeFrame(*frame, record, contentBlock) == 0); */
        /*   if (!firstImage_ && frameValid) { */
        /*     job = make_unique<ImageJob>(vrs::ImageFormat::VIDEO); */
        /*   } */
        /* } else { */
        /*   if (firstImage_) { */
        /*     frameValid = PixelFrame::readFrame(frame, record.reader, contentBlock);
   */
        /*   } else { */
        /*     // decoding & pixel format conversion happen asynchronously */
        /*     job = make_unique<ImageJob>(imageFormat_); */
        /*     job->buffer.resize(contentBlock.getBlockSize()); */
        /*     frameValid = (record.reader->read(job->buffer) == 0); */
        /*   } */
        /* } */
        /* if (frameValid && job) { */
        /*   job->frame = std::move(frame); */
        /*   imageJobs_.startThreadIfNeeded(&FramePlayer::imageJobsThreadActivity,
   * this); */
        /*   imageJobs_.sendJob(std::move(job)); */
        /*   return true; */
        /* } */
        /* if (firstImage_) { */
        /*   fmt::print( */
        /*       "Found '{} - {}': {}, {}", */
        /*       record.streamId.getNumericName(), */
        /*       record.streamId.getTypeName(), */
        /*       getCurrentRecordFormatReader()->recordFormat.asString(), */
        /*       spec.asString()); */
        /*   if (frameValid && spec.getImageFormat() != vrs::ImageFormat::RAW) { */
        /*     fmt::print(" - {}", frame->getSpec().asString()); */
        /*   } */
        /*   blankMode_ = false; */
        /* } */
        /* if (frameValid) { */
        /*   convertFrame(frame); */
        /*   if (firstImage_) { */
        /*     if (needsConvertedFrame_) { */
        /*       fmt::print(" -> {}", frame->getSpec().asString()); */
        /*     } */
        /*     if (estimatedFps_ != 0) { */
        /*       fmt::print(", {} fps", estimatedFps_); */
        /*     } */
        /*     frame->blankFrame(); */
        /*     blankMode_ = true; */
        /*   } */
        /*   widget_->swapImage(frame); */
        /* } */
        /* recycle(frame, !needsConvertedFrame_); */
        /* if (firstImage_) { */
        /*   fmt::print("\n"); */
        /*   firstImage_ = false; */
        /* } */
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
