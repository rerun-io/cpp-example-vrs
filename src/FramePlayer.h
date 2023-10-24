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

#pragma once

#include <memory>

#include <vrs/helpers/JobQueue.h>
#include <vrs/RecordFormat.h>
#include <vrs/utils/PixelFrame.h>
#include <vrs/utils/VideoFrameHandler.h>
#include <vrs/utils/VideoRecordFormatStreamPlayer.h>
#include <rerun.hpp>

/* #include "MetaDataCollector.h" */

enum class FileReaderState;

namespace rerun_vrs {

    struct ImageJob {
        ImageJob(vrs::ImageFormat imageFormat) : imageFormat{imageFormat} {}

        vrs::ImageFormat imageFormat;
        std::shared_ptr<vrs::utils::PixelFrame> frame;
        std::vector<uint8_t> buffer;
    };

    class RerunFramePlayer : public vrs::utils::VideoRecordFormatStreamPlayer {
      public:
        explicit RerunFramePlayer(vrs::StreamId id, rerun::RecordingStream& rec);

        bool onDataLayoutRead(const vrs::CurrentRecord& r, size_t blockIndex, vrs::DataLayout&) override;
        bool onImageRead(const vrs::CurrentRecord& r, size_t blockIndex, const vrs::ContentBlock&) override;

        vrs::StreamId getId() const {
            return id_;
        }

        void setEstimatedFps(int estimatedFps) {
            estimatedFps_ = estimatedFps;
        }

        void imageJobsThreadActivity();

      private:
        rerun::RecordingStream& rec_;
        bool needsConvertedFrame_{false};
        vrs::StreamId id_;
        std::string entityPath_;
        /* MetaDataCollector descriptions_; */
        bool blankMode_{true};
        bool enabled_{true};
        bool firstImage_{true};
        int estimatedFps_;
        /* Fps dataFps_; */
        FileReaderState state_{};

        vrs::JobQueueWithThread<std::unique_ptr<ImageJob>> imageJobs_;

        void convertFrame(std::shared_ptr<vrs::utils::PixelFrame>& frame);
        void makeBlankFrame(std::shared_ptr<vrs::utils::PixelFrame>& frame);
        void recycle(std::shared_ptr<vrs::utils::PixelFrame>& frame, bool inputNotConvertedFrame);
    };

} // namespace rerun_vrs
