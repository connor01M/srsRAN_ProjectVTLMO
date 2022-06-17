/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "du_high.h"
#include "adapters.h"
#include "du_high_executor_strategies.h"
#include "srsgnb/du_manager/du_manager_factory.h"
#include "srsgnb/f1_interface/du/f1ap_du_factory.h"
#include "srsgnb/mac/mac_factory.h"

using namespace srsgnb;
using namespace srs_du;

void assert_du_high_configuration_valid(const du_high_configuration& cfg)
{
  srsran_assert(cfg.du_mng_executor != nullptr, "Invalid DU manager executor");
  srsran_assert(cfg.dl_executors != nullptr, "Invalid DL executor mapper");
  srsran_assert(cfg.ul_executors != nullptr, "Invalid UL executor mapper");
}

/// Cell slot handler that additionally increments the DU high timers.
class du_high_slot_handler final : public mac_cell_slot_handler
{
public:
  du_high_slot_handler(timer_manager& timers_, mac_interface& mac_) : timers(timers_), mac(mac_) {}
  void handle_slot_indication(slot_point sl_tx) override
  {
    // Step timers by one millisecond.
    if (sl_tx.to_uint() % get_nof_slots_per_subframe(to_subcarrier_spacing(sl_tx.numerology())) == 0) {
      timers.tick_all();
    }

    // Handle slot indication in MAC & Scheduler.
    mac.get_slot_handler(to_du_cell_index(0)).handle_slot_indication(sl_tx);
  }

private:
  timer_manager& timers;
  mac_interface& mac;
};

du_high::du_high(const du_high_configuration& config_) : cfg(config_), timers(128)
{
  assert_du_high_configuration_valid(cfg);

  // Create layers
  mac  = create_mac(mac_ev_notifier, *cfg.ul_executors, *cfg.dl_executors, *cfg.du_mng_executor, *cfg.phy_adapter);
  f1ap       = create_f1ap(timers, *cfg.f1c_notifier);
  du_manager = create_du_manager(timers,
                                 mac->get_ue_configurator(),
                                 mac->get_cell_manager(),
                                 *f1ap,
                                 *f1ap,
                                 *f1ap,
                                 rlc_rx_upper_dp,
                                 *cfg.du_mng_executor,
                                 cfg.cells);

  // Connect Layer->DU manager notifiers.
  mac_ev_notifier.connect(*du_manager);

  // Cell slot handler.
  main_cell_slot_handler = std::make_unique<du_high_slot_handler>(timers, *mac);
}

du_high::~du_high()
{
  stop();
}

void du_high::start()
{
  du_manager->start();
}

void du_high::stop() {}

f1c_message_handler& du_high::get_f1c_message_handler()
{
  return *f1ap;
}

mac_pdu_handler& du_high::get_pdu_handler(du_cell_index_t cell_idx)
{
  return mac->get_pdu_handler(cell_idx);
}

mac_cell_slot_handler& du_high::get_slot_handler(du_cell_index_t cell_idx)
{
  if (cell_idx == 0) {
    return *main_cell_slot_handler;
  }
  return mac->get_slot_handler(cell_idx);
}
