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

#include <array>
#include <memory>
#include <sstream>

#include <vrs/DataLayout.h>
#include <vrs/utils/PixelFrame.h>

#include "IMUPlayer.h"
#include "utils.h"

namespace rerun_vrs {

    struct IMUConfigurationDataLayout : public vrs::AutoDataLayout {
        vrs::DataPieceValue<vrs::Bool> hasAccelerometer_{"has_accelerometer"};
        vrs::DataPieceValue<vrs::Bool> hasGyroscope_{"has_gyroscope"};
        vrs::DataPieceValue<vrs::Bool> hasMagnetometer_{"has_magnetometer"};

        vrs::AutoDataLayoutEnd endLayout;
    };

    struct IMUDataDataLayout : public vrs::AutoDataLayout {
        vrs::DataPieceArray<float> accelMSec2{"accelerometer", 3};
        vrs::DataPieceArray<float> gyroRadSec{"gyroscope", 3};
        vrs::DataPieceArray<float> magTesla{"magnetometer", 3};

        vrs::AutoDataLayoutEnd endLayout;
    };

    IMUPlayer::IMUPlayer(vrs::StreamId id, std::shared_ptr<rerun::RecordingStream> rec)
        : id_{id}, rec_{rec}, entityPath_{add_quotes(id.getName())} {}

    bool IMUPlayer::onDataLayoutRead(
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

            // read properties required for logging
            auto& config = getExpectedLayout<IMUConfigurationDataLayout>(layout, blockIndex);
            hasAccelerometer_ = config.hasAccelerometer_.get();
            hasGyroscope_ = config.hasGyroscope_.get();
            hasMagnetometer_ = config.hasMagnetometer_.get();
        }

        if (record.recordType == vrs::Record::Type::DATA) {
            auto& data = getExpectedLayout<IMUDataDataLayout>(layout, blockIndex);

            if (hasAccelerometer_) {
                std::array<float, 3> accelMSec2;
                if (data.accelMSec2.get(accelMSec2.data(), accelMSec2.size()))
                    logAccelerometer(accelMSec2);
            }
            if (hasGyroscope_) {
                std::array<float, 3> gyroRadSec;
                if (data.gyroRadSec.get(gyroRadSec.data(), gyroRadSec.size()))
                    logGyroscope(gyroRadSec);
            }
            if (hasMagnetometer_) {
                std::array<float, 3> magTesla;
                if (data.magTesla.get(magTesla.data(), magTesla.size()))
                    logMagnetometer(magTesla);
            }
        }

        return false;
    }

    void IMUPlayer::logAccelerometer(const std::array<float, 3>& accelMSec2) {
        rec_->log(
            (entityPath_ + "/accelerometer").c_str(),
            rerun::Arrows3D::from_vectors({rerun::datatypes::Vec3D(accelMSec2)})
        );
        rec_->log(
            (entityPath_ + "/accelerometer/x").c_str(),
            rerun::TimeSeriesScalar(accelMSec2[0])
        );
        rec_->log(
            (entityPath_ + "/accelerometer/y").c_str(),
            rerun::TimeSeriesScalar(accelMSec2[1])
        );
        rec_->log(
            (entityPath_ + "/accelerometer/z").c_str(),
            rerun::TimeSeriesScalar(accelMSec2[2])
        );
    }

    void IMUPlayer::logGyroscope(const std::array<float, 3>& gyroRadSec) {
        rec_->log((entityPath_ + "/gyroscope/x").c_str(), rerun::TimeSeriesScalar(gyroRadSec[0]));
        rec_->log((entityPath_ + "/gyroscope/y").c_str(), rerun::TimeSeriesScalar(gyroRadSec[1]));
        rec_->log((entityPath_ + "/gyroscope/z").c_str(), rerun::TimeSeriesScalar(gyroRadSec[2]));
    }

    void IMUPlayer::logMagnetometer(const std::array<float, 3>& magTesla) {
        rec_->log((entityPath_ + "/magnetometer/x").c_str(), rerun::TimeSeriesScalar(magTesla[0]));
        rec_->log((entityPath_ + "/magnetometer/y").c_str(), rerun::TimeSeriesScalar(magTesla[1]));
        rec_->log((entityPath_ + "/magnetometer/z").c_str(), rerun::TimeSeriesScalar(magTesla[2]));
    }

    bool mightContainIMUData(const vrs::StreamId& id) {
        return id.getTypeId() == vrs::RecordableTypeId::SlamImuData ||
               id.getTypeId() == vrs::RecordableTypeId::SlamMagnetometerData ||
               id.getTypeId() == vrs::RecordableTypeId::ImuRecordableClass;
    }
} // namespace rerun_vrs
