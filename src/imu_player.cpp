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

#include "imu_player.hpp"

namespace rerun_vrs {

    struct IMUConfigurationDataLayout : public vrs::AutoDataLayout {
        vrs::DataPieceValue<vrs::Bool> _has_accelerometer{"has_accelerometer"};
        vrs::DataPieceValue<vrs::Bool> _has_gyroscope{"has_gyroscope"};
        vrs::DataPieceValue<vrs::Bool> _has_magnetometer{"has_magnetometer"};

        vrs::AutoDataLayoutEnd endLayout;
    };

    struct IMUDataDataLayout : public vrs::AutoDataLayout {
        vrs::DataPieceArray<float> accelMSec2{"accelerometer", 3};
        vrs::DataPieceArray<float> gyroRadSec{"gyroscope", 3};
        vrs::DataPieceArray<float> magTesla{"magnetometer", 3};

        vrs::AutoDataLayoutEnd endLayout;
    };

    IMUPlayer::IMUPlayer(vrs::StreamId id, std::shared_ptr<const rerun::RecordingStream> rec)
        : _id{id}, _rec{rec}, _entity_path{rerun::new_entity_path({id.getName()})} {}

    bool IMUPlayer::onDataLayoutRead(
        const vrs::CurrentRecord& record, size_t blockIndex, vrs::DataLayout& layout
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

            // read properties required for logging
            auto& config = getExpectedLayout<IMUConfigurationDataLayout>(layout, blockIndex);
            _has_accelerometer = config._has_accelerometer.get();
            _has_gyroscope = config._has_gyroscope.get();
            _has_magnetometer = config._has_magnetometer.get();
        }

        if (record.recordType == vrs::Record::Type::DATA) {
            auto& data = getExpectedLayout<IMUDataDataLayout>(layout, blockIndex);

            if (_has_accelerometer) {
                std::array<float, 3> accelMSec2;
                if (data.accelMSec2.get(accelMSec2.data(), accelMSec2.size())) {
                    log_accelerometer(accelMSec2);
                }
            }
            if (_has_gyroscope) {
                std::array<float, 3> gyroRadSec;
                if (data.gyroRadSec.get(gyroRadSec.data(), gyroRadSec.size())) {
                    log_gyroscope(gyroRadSec);
                }
            }
            if (_has_magnetometer) {
                std::array<float, 3> magTesla;
                if (data.magTesla.get(magTesla.data(), magTesla.size())) {
                    log_magnetometer(magTesla);
                }
            }
        }

        return false;
    }

    void IMUPlayer::log_accelerometer(const std::array<float, 3>& accelMSec2) {
        _rec->log(_entity_path + "/accelerometer", rerun::Arrows3D::from_vectors({accelMSec2}));
        _rec->log(_entity_path + "/accelerometer", rerun::Scalars(accelMSec2));
    }

    void IMUPlayer::log_gyroscope(const std::array<float, 3>& gyroRadSec) {
        _rec->log(_entity_path + "/gyroscope", rerun::Scalars(gyroRadSec));
    }

    void IMUPlayer::log_magnetometer(const std::array<float, 3>& magTesla) {
        _rec->log(_entity_path + "/magnetometer", rerun::Scalars(magTesla));
    }

    bool might_contain_imu_data(const vrs::StreamId& id) {
        return id.getTypeId() == vrs::RecordableTypeId::SlamImuData ||
               id.getTypeId() == vrs::RecordableTypeId::SlamMagnetometerData ||
               id.getTypeId() == vrs::RecordableTypeId::ImuRecordableClass;
    }
} // namespace rerun_vrs
