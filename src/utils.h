#pragma once

#include <string>

namespace rerun_vrs {

    /// Add double quotes around string
    /// Useful to support arbitary strings as part of entity path in Rerun
    std::string add_quotes(const std::string& str);

} // namespace rerun_vrs
