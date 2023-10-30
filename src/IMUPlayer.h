/* Modified from https://github.com/facebookresearch/vrs
 * and https://github.com/facebookresearch/projectaria_tools
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

#include <vrs/DataLayout.h>
#include <vrs/RecordFormat.h>
#include <vrs/StreamId.h>
#include <vrs/StreamPlayer.h>
#include <vrs/utils/VideoRecordFormatStreamPlayer.h>
#include <rerun.hpp>

namespace rerun_vrs {
    class IMUPlayer : public vrs::RecordFormatStreamPlayer {
      public:
        explicit IMUPlayer(vrs::StreamId id, std::shared_ptr<const rerun::RecordingStream> rec);

        bool onDataLayoutRead(const vrs::CurrentRecord& r, size_t blockIndex, vrs::DataLayout&)
            override;

      private:
        void logAccelerometer(const std::array<float, 3>& accelMSec2);
        void logGyroscope(const std::array<float, 3>& gyroRadSec);
        void logMagnetometer(const std::array<float, 3>& magTesla);

        std::shared_ptr<const rerun::RecordingStream> rec_;
        vrs::StreamId id_;
        std::string entityPath_;
        bool enabled_{true};
        bool hasAccelerometer_;
        bool hasGyroscope_;
        bool hasMagnetometer_;
    };

    bool mightContainIMUData(const vrs::StreamId& id);

} // namespace rerun_vrs
