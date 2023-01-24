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

#include "../common/e1_types.h"
#include "srsgnb/adt/expected.h"
#include "srsgnb/asn1/e1ap/e1ap.h"
#include "srsgnb/cu_up/cu_up_types.h"
#include "srsgnb/e1/common/e1_common.h"

namespace srsgnb {
namespace srs_cu_up {

/// \brief Request to create a new UE and bearer context.
struct e1ap_bearer_context_setup_request {
  asn1::e1ap::sys_bearer_context_setup_request_c request;
  asn1::fixed_octstring<3U, true>                serving_plmn;
  // TODO: add optional fields like DU-ID, etc
};

/// \brief Response to a bearer context setup request including UE index for E1 map.
struct e1ap_bearer_context_setup_response {
  bool                                        success;
  ue_index_t                                  ue_index = INVALID_UE_INDEX; // Valid UE index if setup was successful.
  asn1::e1ap::sys_bearer_context_setup_resp_c sys_bearer_context_setup_resp;
  asn1::e1ap::cause_c                         cause; // Cause if setup was unsuccessful.
};

/// \brief Request to modify a bearer context.
struct e1ap_bearer_context_modification_request {
  ue_index_t                                   ue_index = INVALID_UE_INDEX;
  asn1::e1ap::sys_bearer_context_mod_request_c request;
  // TODO: add optional fields
};

/// \brief Response to a bearer context modification request including UE index for E1 map.
struct e1ap_bearer_context_modification_response {
  bool       success;
  ue_index_t ue_index = INVALID_UE_INDEX; // Valid UE index if modification was successful.
  asn1::e1ap::sys_bearer_context_mod_resp_c sys_bearer_context_modification_resp;
  asn1::e1ap::cause_c                       cause; // Cause if modification was unsuccessful.
};

struct e1ap_ue_context {
  gnb_cu_cp_ue_e1ap_id_t cu_cp_e1_ue_id = gnb_cu_cp_ue_e1ap_id_t::invalid;
  ue_index_t             ue_index       = INVALID_UE_INDEX;
};

/// Handle E1 interface management procedures as defined in TS 38.463 section 8.2.
class e1_connection_manager
{
public:
  virtual ~e1_connection_manager() = default;

  /// \brief Creates and transmits the CU-CP initiated E1 Setup outcome to the CU-CP.
  virtual void handle_cu_cp_e1_setup_response(const cu_cp_e1_setup_response& msg) = 0;
};

/// Methods used by E1 to notify the CU-CP (manager).
class e1_cu_cp_notifier
{
public:
  virtual ~e1_cu_cp_notifier() = default;

  /// \brief Notifies about the reception of a E1 Setup Request message.
  virtual void on_e1_setup_request_received(const cu_cp_e1_setup_request& msg) = 0;
};

/// Methods used by E1 to notify the CU-UP.
class e1ap_cu_cp_notifier
{
public:
  virtual ~e1ap_cu_cp_notifier() = default;

  /// \brief Notifies about the reception of a E1 Setup Request message.
  /// \param[in] msg The received E1 Setup Request message.
  /// \return The CU-CP E1 Setup Response message.
  virtual cu_cp_e1_setup_response on_cu_cp_e1_setup_request_received(const cu_cp_e1_setup_request& msg) = 0;

  /// \brief Notifies the UE manager to create a UE context.
  /// \param[in] msg The received bearer context setup message.
  /// \return Returns a bearer context response message containing the index of the created UE context.
  virtual e1ap_bearer_context_setup_response
  on_bearer_context_setup_request_received(const e1ap_bearer_context_setup_request& msg) = 0;

  /// \brief Notifies the UE manager to create a UE context.
  /// \param[in] msg The received bearer context modification message.
  /// \return Returns a bearer context response message containing the index of the created UE context.
  virtual e1ap_bearer_context_modification_response
  on_bearer_context_modification_request_received(const e1ap_bearer_context_modification_request& msg) = 0;
};

/// Combined entry point for E1 handling.
class e1_interface : public e1_message_handler, public e1_event_handler, public e1_connection_manager
{
public:
  virtual ~e1_interface() = default;
};

} // namespace srs_cu_up
} // namespace srsgnb
