/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#ifndef SRSGNB_PHY_UPPER_SIGNAL_PROCESSORS_DMRS_PBCH_PROCESSOR_H
#define SRSGNB_PHY_UPPER_SIGNAL_PROCESSORS_DMRS_PBCH_PROCESSOR_H

#include "srsgnb/adt/static_vector.h"
#include "srsgnb/phy/resource_grid.h"

namespace srsgnb {

/// Describes a DMRS for PBCH processor interface
class dmrs_pbch_processor
{
public:
  /// Describes the required parameters to generate the signal
  struct config_t {
    /// Physical cell identifier
    unsigned phys_cell_id;
    /// SS/PBCH block index in the burst
    unsigned ssb_idx;
    /// Maximum number of SS/PBCH transmissions in a burst (5ms)
    unsigned L_max;
    /// First subcarrier in the resource grid
    unsigned ssb_first_subcarrier;
    /// Denotes the first symbol of the SS/PBCH block within the slot.
    unsigned ssb_first_symbol;
    /// Half frame flag
    bool n_hf;
    /// Linear signal amplitude
    float amplitude;
    /// Port indexes to map the signal.
    static_vector<uint8_t, MAX_PORTS> ports;
  };

  /// Default destructor
  virtual ~dmrs_pbch_processor() = default;

  /// \brief Generates and maps DMRS for PBCH
  /// \param [out] grid Provides the destination resource grid
  /// \param [in] config Provides the required configuration to generate and map the signal
  virtual void map(resource_grid_writer& grid, const config_t& config) = 0;
};

} // namespace srsgnb

#endif // SRSGNB_PHY_UPPER_SIGNAL_PROCESSORS_DMRS_PBCH_PROCESSOR_H
