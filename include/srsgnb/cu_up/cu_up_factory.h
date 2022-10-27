/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsgnb/cu_up/cu_up.h"
#include "srsgnb/cu_up/cu_up_configuration.h"
#include <memory>

namespace srsgnb {

/// Creates an instance of an CU-UP.
std::unique_ptr<srs_cu_up::cu_up_interface> create_cu_up(const srs_cu_up::cu_up_configuration& cfg_);

} // namespace srsgnb