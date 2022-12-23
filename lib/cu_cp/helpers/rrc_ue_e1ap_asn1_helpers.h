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

#include "../converters/e1ap_asn1_converters.h"

namespace srsgnb {
namespace srs_cu_cp {

inline void fill_asn1_bearer_context_setup_request(asn1::e1ap::bearer_context_setup_request_s e1ap_request,
                                                   const e1ap_bearer_context_setup_request&   request)
{
  // security info
  e1ap_request->security_info.value.security_algorithm.ciphering_algorithm =
      ciphering_algorithm_to_e1ap_asn1(request.security_info.security_algorithm.ciphering_algo);
  if (request.security_info.security_algorithm.integrity_protection_algorithm.has_value()) {
    e1ap_request->security_info.value.security_algorithm.integrity_protection_algorithm_present = true;
    e1ap_request->security_info.value.security_algorithm.integrity_protection_algorithm =
        integrity_algorithm_to_e1ap_asn1(
            request.security_info.security_algorithm.integrity_protection_algorithm.value());
  }
  e1ap_request->security_info.value.upsecuritykey.encryption_key =
      request.security_info.upsecuritykey.encryption_key.copy();
  e1ap_request->security_info.value.upsecuritykey.integrity_protection_key =
      request.security_info.upsecuritykey.integrity_protection_key.copy();

  // ue dl aggregate maximum bit rate
  e1ap_request->uedl_aggregate_maximum_bit_rate.value = request.uedl_aggregate_maximum_bit_rate;

  // serving plmn
  e1ap_request->serving_plmn.value.from_string(request.serving_plmn);

  // activity notification level
  asn1::string_to_enum(e1ap_request->activity_notif_level.value, request.activity_notif_level);

  // pdu session resource to setup list
  e1ap_request->sys_bearer_context_setup_request.value.set_ng_ran_bearer_context_setup_request();
  auto& ng_ran_bearer_context_setup_request =
      e1ap_request->sys_bearer_context_setup_request.value.ng_ran_bearer_context_setup_request();

  for (auto pdu_session_res_item : request.pdu_session_res_to_setup_list) {
    asn1::protocol_ie_field_s<asn1::e1ap::ng_ran_bearer_context_setup_request_o> bearer_request_item;
    auto& e1ap_pdu_session_res_items = bearer_request_item.value().pdu_session_res_to_setup_list();

    asn1::e1ap::pdu_session_res_to_setup_item_s e1ap_pdu_session_res_item;

    // pdu session id
    e1ap_pdu_session_res_item.pdu_session_id = pdu_session_res_item.pdu_session_id;

    // pdu session type
    asn1::string_to_enum(e1ap_pdu_session_res_item.pdu_session_type, pdu_session_res_item.pdu_session_type);

    // s-nssai
    e1ap_pdu_session_res_item.snssai = snssai_to_e1ap_asn1(pdu_session_res_item.snssai);

    // ng ul up transport layer information
    up_transport_layer_info_to_asn1(e1ap_pdu_session_res_item.ng_ul_up_tnl_info,
                                    pdu_session_res_item.ng_ul_up_tnl_info);

    // security indication
    asn1::string_to_enum(e1ap_pdu_session_res_item.security_ind.confidentiality_protection_ind,
                         pdu_session_res_item.security_ind.confidentiality_protection_ind);
    asn1::string_to_enum(e1ap_pdu_session_res_item.security_ind.integrity_protection_ind,
                         pdu_session_res_item.security_ind.integrity_protection_ind);
    if (pdu_session_res_item.security_ind.maximum_ipdatarate.has_value()) {
      e1ap_pdu_session_res_item.security_ind.maximum_ipdatarate_present = true;
      asn1::string_to_enum(e1ap_pdu_session_res_item.security_ind.maximum_ipdatarate.max_iprate,
                           pdu_session_res_item.security_ind.maximum_ipdatarate.value());
    }

    // drb to setup list ng ran
    for (const auto& drb_to_setup_item : pdu_session_res_item.drb_to_setup_list_ng_ran) {
      asn1::e1ap::drb_to_setup_item_ng_ran_s e1ap_drb_to_setup_item;

      e1ap_drb_to_setup_item.drb_id = drb_id_to_uint(drb_to_setup_item.drb_id);

      // sdap config
      e1ap_drb_to_setup_item.sdap_cfg = sdap_config_to_e1ap_asn1(drb_to_setup_item.sdap_cfg);

      // pdcp config
      e1ap_drb_to_setup_item.pdcp_cfg = pdcp_config_to_e1ap_asn1(drb_to_setup_item.pdcp_cfg);

      // cell group info
      for (const auto& cell_group_info_item : drb_to_setup_item.cell_group_info) {
        asn1::e1ap::cell_group_info_item_s e1ap_cell_group_info_item;

        e1ap_cell_group_info_item.cell_group_id = cell_group_info_item.cell_group_id;

        if (cell_group_info_item.ul_cfg.has_value()) {
          e1ap_cell_group_info_item.ul_cfg_present = true;
          asn1::string_to_enum(e1ap_cell_group_info_item.ul_cfg, cell_group_info_item.ul_cfg.value());
        }

        if (cell_group_info_item.dl_tx_stop.has_value()) {
          e1ap_cell_group_info_item.dl_tx_stop_present = true;
          asn1::string_to_enum(e1ap_cell_group_info_item.dl_tx_stop, cell_group_info_item.dl_tx_stop.value());
        }

        if (cell_group_info_item.rat_type.has_value()) {
          e1ap_cell_group_info_item.rat_type_present = true;
          asn1::string_to_enum(e1ap_cell_group_info_item.rat_type, cell_group_info_item.rat_type.value());
        }

        e1ap_drb_to_setup_item.cell_group_info.push_back(e1ap_cell_group_info_item);
      }

      // qos flow info to be setup
      for (const auto& qos_flow_info_item : drb_to_setup_item.qos_flow_info_to_be_setup) {
        asn1::e1ap::qo_s_flow_qos_param_item_s e1ap_qos_flow_info_item;

        e1ap_qos_flow_info_item.qo_s_flow_id = qos_flow_info_item.qos_flow_id;

        // qos flow level qos params
        auto& qos_flow_level_params = qos_flow_info_item.qos_flow_level_qos_params;

        // dynamic 5qi
        if (qos_flow_level_params.qos_characteristics.dynamic_minus5_qi.has_value()) {
          auto& dynamic_5qi = qos_flow_level_params.qos_characteristics.dynamic_minus5_qi.value();
          e1ap_qos_flow_info_item.qo_sflow_level_qos_params.qo_s_characteristics.set_dynamic_minus5_qi();
          auto& e1ap_dynamic_5qi =
              e1ap_qos_flow_info_item.qo_sflow_level_qos_params.qo_s_characteristics.dynamic_minus5_qi();

          e1ap_dynamic_5qi.qo_sprio_level                 = dynamic_5qi.qos_prio_level;
          e1ap_dynamic_5qi.packet_delay_budget            = dynamic_5qi.packet_delay_budget;
          e1ap_dynamic_5qi.packet_error_rate.per_scalar   = dynamic_5qi.packet_error_rate.per_scalar;
          e1ap_dynamic_5qi.packet_error_rate.per_exponent = dynamic_5qi.packet_error_rate.per_exponent;
          if (dynamic_5qi.five_qi.has_value()) {
            e1ap_dynamic_5qi.five_qi_present = true;
            e1ap_dynamic_5qi.five_qi         = dynamic_5qi.five_qi.value();
          }
          if (dynamic_5qi.delay_crit.has_value()) {
            e1ap_dynamic_5qi.delay_crit_present = true;
            asn1::string_to_enum(e1ap_dynamic_5qi.delay_crit, dynamic_5qi.delay_crit.value());
          }
          if (dynamic_5qi.averaging_win.has_value()) {
            e1ap_dynamic_5qi.averaging_win_present = true;
            e1ap_dynamic_5qi.averaging_win         = dynamic_5qi.averaging_win.value();
          }
          if (dynamic_5qi.max_data_burst_volume.has_value()) {
            e1ap_dynamic_5qi.max_data_burst_volume_present = true;
            e1ap_dynamic_5qi.max_data_burst_volume         = dynamic_5qi.max_data_burst_volume.value();
          }
        } else /* non dynamic 5qi */ {
          auto& non_dynamic_5qi = qos_flow_level_params.qos_characteristics.non_dynamic_minus5_qi.value();
          e1ap_qos_flow_info_item.qo_sflow_level_qos_params.qo_s_characteristics.set_non_dynamic_minus5_qi();
          auto& e1ap_non_dynamic_5qi =
              e1ap_qos_flow_info_item.qo_sflow_level_qos_params.qo_s_characteristics.non_dynamic_minus5_qi();

          e1ap_non_dynamic_5qi.five_qi = non_dynamic_5qi.five_qi;

          if (non_dynamic_5qi.qos_prio_level.has_value()) {
            e1ap_non_dynamic_5qi.qo_sprio_level_present = true;
            e1ap_non_dynamic_5qi.qo_sprio_level         = non_dynamic_5qi.qos_prio_level.value();
          }
          if (non_dynamic_5qi.averaging_win.has_value()) {
            e1ap_non_dynamic_5qi.averaging_win_present = true;
            e1ap_non_dynamic_5qi.averaging_win         = non_dynamic_5qi.averaging_win.value();
          }
          if (non_dynamic_5qi.max_data_burst_volume.has_value()) {
            e1ap_non_dynamic_5qi.max_data_burst_volume_present = true;
            e1ap_non_dynamic_5qi.max_data_burst_volume         = non_dynamic_5qi.max_data_burst_volume.value();
          }
        }

        // ng ran alloc retention prio
        e1ap_qos_flow_info_item.qo_sflow_level_qos_params.ngra_nalloc_retention_prio.prio_level =
            qos_flow_level_params.ng_ran_alloc_retention_prio.prio_level;
        asn1::string_to_enum(
            e1ap_qos_flow_info_item.qo_sflow_level_qos_params.ngra_nalloc_retention_prio.pre_emption_cap,
            qos_flow_level_params.ng_ran_alloc_retention_prio.pre_emption_cap);
        asn1::string_to_enum(
            e1ap_qos_flow_info_item.qo_sflow_level_qos_params.ngra_nalloc_retention_prio.pre_emption_vulnerability,
            qos_flow_level_params.ng_ran_alloc_retention_prio.pre_emption_vulnerability);

        // gbr qos flow info
        if (qos_flow_level_params.gbr_qos_flow_info.has_value()) {
          e1ap_qos_flow_info_item.qo_sflow_level_qos_params.gbr_qos_flow_info_present = true;
          e1ap_qos_flow_info_item.qo_sflow_level_qos_params.gbr_qos_flow_info.max_flow_bit_rate_dl =
              qos_flow_level_params.gbr_qos_flow_info.value().max_flow_bit_rate_dl;
          e1ap_qos_flow_info_item.qo_sflow_level_qos_params.gbr_qos_flow_info.max_flow_bit_rate_ul =
              qos_flow_level_params.gbr_qos_flow_info.value().max_flow_bit_rate_ul;
          e1ap_qos_flow_info_item.qo_sflow_level_qos_params.gbr_qos_flow_info.guaranteed_flow_bit_rate_dl =
              qos_flow_level_params.gbr_qos_flow_info.value().guaranteed_flow_bit_rate_dl;
          e1ap_qos_flow_info_item.qo_sflow_level_qos_params.gbr_qos_flow_info.guaranteed_flow_bit_rate_ul =
              qos_flow_level_params.gbr_qos_flow_info.value().guaranteed_flow_bit_rate_ul;
          if (qos_flow_level_params.gbr_qos_flow_info.value().max_packet_loss_rate_dl.has_value()) {
            e1ap_qos_flow_info_item.qo_sflow_level_qos_params.gbr_qos_flow_info.max_packet_loss_rate_dl_present = true;
            e1ap_qos_flow_info_item.qo_sflow_level_qos_params.gbr_qos_flow_info.max_packet_loss_rate_dl =
                qos_flow_level_params.gbr_qos_flow_info.value().max_packet_loss_rate_dl.value();
          }
          if (qos_flow_level_params.gbr_qos_flow_info.value().max_packet_loss_rate_ul.has_value()) {
            e1ap_qos_flow_info_item.qo_sflow_level_qos_params.gbr_qos_flow_info.max_packet_loss_rate_ul_present = true;
            e1ap_qos_flow_info_item.qo_sflow_level_qos_params.gbr_qos_flow_info.max_packet_loss_rate_ul =
                qos_flow_level_params.gbr_qos_flow_info.value().max_packet_loss_rate_ul.value();
          }
        }

        // reflective qos attribute
        if (qos_flow_level_params.reflective_qos_attribute.has_value()) {
          e1ap_qos_flow_info_item.qo_sflow_level_qos_params.reflective_qos_attribute_present = true;
          asn1::string_to_enum(e1ap_qos_flow_info_item.qo_sflow_level_qos_params.reflective_qos_attribute,
                               qos_flow_level_params.reflective_qos_attribute.value());
        }

        // add qos info
        if (qos_flow_level_params.add_qos_info.has_value()) {
          e1ap_qos_flow_info_item.qo_sflow_level_qos_params.add_qos_info_present = true;
          asn1::string_to_enum(e1ap_qos_flow_info_item.qo_sflow_level_qos_params.add_qos_info,
                               qos_flow_level_params.add_qos_info.value());
        }

        // paging policy ind
        if (qos_flow_level_params.paging_policy_ind.has_value()) {
          e1ap_qos_flow_info_item.qo_sflow_level_qos_params.paging_policy_ind_present = true;
          e1ap_qos_flow_info_item.qo_sflow_level_qos_params.paging_policy_ind =
              qos_flow_level_params.paging_policy_ind.value();
        }

        // reflective qos ind
        if (qos_flow_level_params.reflective_qos_ind.has_value()) {
          e1ap_qos_flow_info_item.qo_sflow_level_qos_params.reflective_qos_ind_present = true;
          asn1::string_to_enum(e1ap_qos_flow_info_item.qo_sflow_level_qos_params.reflective_qos_ind,
                               qos_flow_level_params.reflective_qos_ind.value());
        }

        // qos flow map ind
        if (qos_flow_info_item.qos_flow_map_ind.has_value()) {
          e1ap_qos_flow_info_item.qo_sflow_map_ind_present = true;
          asn1::string_to_enum(e1ap_qos_flow_info_item.qo_sflow_map_ind, qos_flow_info_item.qos_flow_map_ind.value());
        }

        e1ap_drb_to_setup_item.qos_flow_info_to_be_setup.push_back(e1ap_qos_flow_info_item);
      }

      // drb data forwarding info request
      if (drb_to_setup_item.drb_data_forwarding_info_request.has_value()) {
        e1ap_drb_to_setup_item.drb_data_forwarding_info_request_present = true;
        asn1::string_to_enum(e1ap_drb_to_setup_item.drb_data_forwarding_info_request.data_forwarding_request,
                             drb_to_setup_item.drb_data_forwarding_info_request.value().data_forwarding_request);
        for (const auto& qos_flow_map_item :
             drb_to_setup_item.drb_data_forwarding_info_request.value().qos_flows_forwarded_on_fwd_tunnels) {
          asn1::e1ap::qo_s_flow_map_item_s e1ap_qos_flow_map_item;
          e1ap_qos_flow_map_item.qo_s_flow_id = qos_flow_map_item.qos_flow_id;
          if (qos_flow_map_item.qos_flow_map_ind.has_value()) {
            e1ap_qos_flow_map_item.qo_sflow_map_ind_present = true;
            asn1::string_to_enum(e1ap_qos_flow_map_item.qo_sflow_map_ind, qos_flow_map_item.qos_flow_map_ind.value());
          }

          e1ap_drb_to_setup_item.drb_data_forwarding_info_request.qo_s_flows_forwarded_on_fwd_tunnels.push_back(
              e1ap_qos_flow_map_item);
        }
      }

      // drb inactivity timer
      if (drb_to_setup_item.drb_inactivity_timer.has_value()) {
        e1ap_drb_to_setup_item.drb_inactivity_timer_present = true;
        asn1::number_to_enum(e1ap_drb_to_setup_item.drb_inactivity_timer,
                             drb_to_setup_item.drb_inactivity_timer.value());
      }

      // pdcp sn status info
      if (drb_to_setup_item.pdcp_sn_status_info.has_value()) {
        e1ap_drb_to_setup_item.pdcp_sn_status_info_present = true;

        e1ap_drb_to_setup_item.pdcp_sn_status_info.pdcp_status_transfer_ul.count_value.pdcp_sn =
            drb_to_setup_item.pdcp_sn_status_info.value().pdcp_status_transfer_ul.count_value.pdcp_sn;
        e1ap_drb_to_setup_item.pdcp_sn_status_info.pdcp_status_transfer_ul.count_value.hfn =
            drb_to_setup_item.pdcp_sn_status_info.value().pdcp_status_transfer_ul.count_value.hfn;
        if (drb_to_setup_item.pdcp_sn_status_info.value()
                .pdcp_status_transfer_ul.receive_statusof_pdcpsdu.has_value()) {
          e1ap_drb_to_setup_item.pdcp_sn_status_info.pdcp_status_transfer_ul.receive_statusof_pdcpsdu_present = true;
          e1ap_drb_to_setup_item.pdcp_sn_status_info.pdcp_status_transfer_ul.receive_statusof_pdcpsdu.from_number(
              drb_to_setup_item.pdcp_sn_status_info.value().pdcp_status_transfer_ul.receive_statusof_pdcpsdu.value());
        }

        e1ap_drb_to_setup_item.pdcp_sn_status_info.pdcp_status_transfer_dl.pdcp_sn =
            drb_to_setup_item.pdcp_sn_status_info.value().pdcp_status_transfer_dl.pdcp_sn;
        e1ap_drb_to_setup_item.pdcp_sn_status_info.pdcp_status_transfer_dl.hfn =
            drb_to_setup_item.pdcp_sn_status_info.value().pdcp_status_transfer_dl.hfn;
      }

      e1ap_pdu_session_res_item.drb_to_setup_list_ng_ran.push_back(e1ap_drb_to_setup_item);
    }

    // pdu session resource dl aggregate maximum bit rate
    if (pdu_session_res_item.pdu_session_res_dl_ambr.has_value()) {
      e1ap_pdu_session_res_item.pdu_session_res_dl_ambr_present = true;
      e1ap_pdu_session_res_item.pdu_session_res_dl_ambr         = pdu_session_res_item.pdu_session_res_dl_ambr.value();
    }

    // pdu session data forwarding info request
    if (pdu_session_res_item.pdu_session_data_forwarding_info_request.has_value()) {
      e1ap_pdu_session_res_item.pdu_session_data_forwarding_info_request_present = true;

      asn1::string_to_enum(
          e1ap_pdu_session_res_item.pdu_session_data_forwarding_info_request.data_forwarding_request,
          pdu_session_res_item.pdu_session_data_forwarding_info_request.value().data_forwarding_request);
      for (const auto& qos_flow_map_item :
           pdu_session_res_item.pdu_session_data_forwarding_info_request.value().qos_flows_forwarded_on_fwd_tunnels) {
        asn1::e1ap::qo_s_flow_map_item_s e1ap_qos_flow_map_item;
        e1ap_qos_flow_map_item.qo_s_flow_id = qos_flow_map_item.qos_flow_id;
        if (qos_flow_map_item.qos_flow_map_ind.has_value()) {
          e1ap_qos_flow_map_item.qo_sflow_map_ind_present = true;
          asn1::string_to_enum(e1ap_qos_flow_map_item.qo_sflow_map_ind, qos_flow_map_item.qos_flow_map_ind.value());
        }

        e1ap_pdu_session_res_item.pdu_session_data_forwarding_info_request.qo_s_flows_forwarded_on_fwd_tunnels
            .push_back(e1ap_qos_flow_map_item);
      }
    }

    // pdu session inactivity timer
    if (pdu_session_res_item.pdu_session_inactivity_timer.has_value()) {
      e1ap_pdu_session_res_item.pdu_session_inactivity_timer_present = true;
      asn1::number_to_enum(e1ap_pdu_session_res_item.pdu_session_inactivity_timer,
                           pdu_session_res_item.pdu_session_inactivity_timer.value());
    }

    // existing allocated ng dl up tnl info
    if (pdu_session_res_item.existing_allocated_ng_dl_up_tnl_info.has_value()) {
      e1ap_pdu_session_res_item.existing_allocated_ng_dl_up_tnl_info_present = true;
      up_transport_layer_info_to_asn1(e1ap_pdu_session_res_item.existing_allocated_ng_dl_up_tnl_info,
                                      pdu_session_res_item.existing_allocated_ng_dl_up_tnl_info.value());
    }

    // network instance
    if (pdu_session_res_item.network_instance.has_value()) {
      e1ap_pdu_session_res_item.network_instance_present = true;
      asn1::number_to_enum(e1ap_pdu_session_res_item.network_instance, pdu_session_res_item.network_instance.value());
    }

    e1ap_pdu_session_res_items.push_back(e1ap_pdu_session_res_item);

    ng_ran_bearer_context_setup_request.push_back(bearer_request_item);
  }

  // ue dl maximum integrity protection data rate
  if (request.uedl_maximum_integrity_protected_data_rate.has_value()) {
    e1ap_request->uedl_maximum_integrity_protected_data_rate_present = true;
    e1ap_request->uedl_maximum_integrity_protected_data_rate.value =
        request.uedl_maximum_integrity_protected_data_rate.value();
  }

  // ue inactivity timer
  if (request.ue_inactivity_timer.has_value()) {
    e1ap_request->ue_inactivity_timer_present = true;
    e1ap_request->ue_inactivity_timer.value   = request.ue_inactivity_timer.value();
  }

  // bearer context status change
  if (request.bearer_context_status_change.has_value()) {
    e1ap_request->bearer_context_status_change_present = true;
    asn1::string_to_enum(e1ap_request->bearer_context_status_change.value,
                         request.bearer_context_status_change.value());
  }

  // ran ue id
  if (request.ranueid.has_value()) {
    e1ap_request->ranueid_present = true;
    e1ap_request->ranueid.value.from_number(ran_ue_id_to_uint(request.ranueid.value()));
  }

  // gnb du id
  if (request.gnb_du_id.has_value()) {
    e1ap_request->gnb_du_id_present = true;
    e1ap_request->gnb_du_id.value   = request.gnb_du_id.value();
  }
}

inline void
fill_rrc_ue_bearer_context_setup_response(rrc_ue_bearer_context_setup_response_message& res,
                                          const e1ap_bearer_context_setup_response&     e1_bearer_context_setup_resp)
{
  if (e1_bearer_context_setup_resp.success) {
    auto& bearer_context_setup_response =
        e1_bearer_context_setup_resp.response->sys_bearer_context_setup_resp->ng_ran_bearer_context_setup_resp();

    for (auto e1ap_res_setup_item : bearer_context_setup_response.pdu_session_res_setup_list.value) {
      rrc_ue_pdu_session_resource_setup_modification_item res_setup_item;
      res_setup_item.pdu_session_id = e1ap_res_setup_item.pdu_session_id;

      // Add NG DL UP TNL Info
      res_setup_item.ng_dl_up_tnl_info = asn1_to_up_transport_layer_info(e1ap_res_setup_item.ng_dl_up_tnl_info);

      // Add DRB Setup List NG RAN
      for (auto e1ap_drb_setup_item : e1ap_res_setup_item.drb_setup_list_ng_ran) {
        rrc_ue_drb_setup_item_ng_ran drb_setup_item;
        drb_setup_item.drb_id = uint_to_drb_id(e1ap_drb_setup_item.drb_id);

        // Add UL UP Transport Params
        for (auto ul_up_transport_param : e1ap_drb_setup_item.ul_up_transport_params) {
          rrc_ue_up_params_item ue_params_item;
          ue_params_item.up_tnl_info   = asn1_to_up_transport_layer_info(e1ap_res_setup_item.ng_dl_up_tnl_info);
          ue_params_item.cell_group_id = ul_up_transport_param.cell_group_id;

          drb_setup_item.ul_up_transport_params.push_back(ue_params_item);
        }

        // Add Flow setup List
        for (auto e1ap_qos_flow_item : e1ap_drb_setup_item.flow_setup_list) {
          rrc_ue_qos_flow_item qos_flow_item;
          qos_flow_item.qos_flow_id = e1ap_qos_flow_item.qo_s_flow_id;

          drb_setup_item.flow_setup_list.push_back(qos_flow_item);
        }

        // Add Flow Failed List
        for (auto e1ap_failed_qos_flow_item : e1ap_drb_setup_item.flow_failed_list) {
          rrc_ue_qos_flow_failed_item failed_qos_flow_item;

          failed_qos_flow_item.qos_flow_id = e1ap_failed_qos_flow_item.qo_s_flow_id;
          failed_qos_flow_item.cause       = e1ap_cause_to_cu_cp_cause(e1ap_failed_qos_flow_item.cause);

          drb_setup_item.flow_failed_list.push_back(failed_qos_flow_item);
        }

        // Add DRB Data Forwarding Info Response
        if (e1ap_drb_setup_item.drb_data_forwarding_info_resp_present) {
          if (e1ap_drb_setup_item.drb_data_forwarding_info_resp.ul_data_forwarding_present) {
            drb_setup_item.drb_data_forwarding_info_resp.value().ul_data_forwarding =
                asn1_to_up_transport_layer_info(e1ap_drb_setup_item.drb_data_forwarding_info_resp.ul_data_forwarding);
          }
          if (e1ap_drb_setup_item.drb_data_forwarding_info_resp.dl_data_forwarding_present) {
            drb_setup_item.drb_data_forwarding_info_resp.value().dl_data_forwarding =
                asn1_to_up_transport_layer_info(e1ap_drb_setup_item.drb_data_forwarding_info_resp.dl_data_forwarding);
          }
        }

        res_setup_item.drb_setup_list_ng_ran.push_back(drb_setup_item);
      }

      // Add DRB Failed List NG RAN
      for (auto e1ap_drb_failed_item : e1ap_res_setup_item.drb_failed_list_ng_ran) {
        rrc_ue_drb_failed_item_ng_ran drb_failed_item;
        drb_failed_item.drb_id = uint_to_drb_id(e1ap_drb_failed_item.drb_id);
        drb_failed_item.cause  = e1ap_cause_to_cu_cp_cause(e1ap_drb_failed_item.cause);

        res_setup_item.drb_failed_list_ng_ran.push_back(drb_failed_item);
      }

      // Add Security Result
      if (e1ap_res_setup_item.security_result_present) {
        res_setup_item.security_result.value().confidentiality_protection_result =
            e1ap_res_setup_item.security_result.confidentiality_protection_result.to_string();
        res_setup_item.security_result.value().integrity_protection_result =
            e1ap_res_setup_item.security_result.integrity_protection_result.to_string();
      }

      // Add PDU Session Data Forwarding Info Response
      if (e1ap_res_setup_item.pdu_session_data_forwarding_info_resp_present) {
        if (e1ap_res_setup_item.pdu_session_data_forwarding_info_resp.ul_data_forwarding_present) {
          res_setup_item.pdu_session_data_forwarding_info_resp.value().ul_data_forwarding =
              asn1_to_up_transport_layer_info(
                  e1ap_res_setup_item.pdu_session_data_forwarding_info_resp.ul_data_forwarding);
        }
        if (e1ap_res_setup_item.pdu_session_data_forwarding_info_resp.dl_data_forwarding_present) {
          res_setup_item.pdu_session_data_forwarding_info_resp.value().dl_data_forwarding =
              asn1_to_up_transport_layer_info(
                  e1ap_res_setup_item.pdu_session_data_forwarding_info_resp.dl_data_forwarding);
        }
      }

      // Add NG DL UP Unchanged
      if (e1ap_res_setup_item.ng_dl_up_unchanged_present) {
        res_setup_item.ng_dl_up_unchanged.value() =
            e1ap_asn1_ng_dl_up_unchanged_to_bool(e1ap_res_setup_item.ng_dl_up_unchanged);
      }

      res.pdu_session_resource_setup_list.push_back(res_setup_item);
    }

    // Add pdu session res failed list
    if (bearer_context_setup_response.pdu_session_res_failed_list_present) {
      for (auto e1ap_failed_item : bearer_context_setup_response.pdu_session_res_failed_list.value) {
        rrc_ue_pdu_session_resource_failed_item failed_item;

        failed_item.pdu_session_id = e1ap_failed_item.pdu_session_id;
        failed_item.cause          = e1ap_cause_to_cu_cp_cause(e1ap_failed_item.cause);

        res.pdu_session_resource_failed_list.push_back(failed_item);
      }
    }
  } else {
    res.success = false;
    res.cause   = e1ap_cause_to_cu_cp_cause(e1_bearer_context_setup_resp.failure->cause.value);
    if (e1_bearer_context_setup_resp.failure->crit_diagnostics_present) {
      // TODO: Add crit diagnostics
    }
  }
}

inline void fill_asn1_bearer_context_modification_request(e1ap_bearer_context_modification_request& e1_request,
                                                          const rrc_ue_bearer_context_modification_request_message& msg)
{
  // TODO: Add needed info to input struct and fill msg
}

inline void fill_rrc_ue_bearer_context_modification_response(
    rrc_ue_bearer_context_modification_response_message& res,
    const e1ap_bearer_context_modification_response&     e1_bearer_context_modification_resp)
{
  if (e1_bearer_context_modification_resp.success) {
    // Add NG RAN bearer context modification response
    if (e1_bearer_context_modification_resp.response->sys_bearer_context_mod_resp_present) {
      auto& bearer_context_mod_response =
          e1_bearer_context_modification_resp.response->sys_bearer_context_mod_resp->ng_ran_bearer_context_mod_resp();

      // Add PDU session resource setup list
      if (bearer_context_mod_response.pdu_session_res_setup_mod_list_present) {
        for (auto e1ap_res_mod_item : bearer_context_mod_response.pdu_session_res_setup_mod_list.value) {
          rrc_ue_pdu_session_resource_setup_modification_item res_mod_item;
          res_mod_item.pdu_session_id = e1ap_res_mod_item.pdu_session_id;

          // Add NG DL UP TNL Info
          res_mod_item.ng_dl_up_tnl_info = asn1_to_up_transport_layer_info(e1ap_res_mod_item.ng_dl_up_tnl_info);

          // Add DRB Setup List NG RAN
          for (auto e1ap_drb_setup_item : e1ap_res_mod_item.drb_setup_mod_list_ng_ran) {
            rrc_ue_drb_setup_item_ng_ran drb_setup_item;
            drb_setup_item.drb_id = uint_to_drb_id(e1ap_drb_setup_item.drb_id);

            // Add UL UP Transport Params
            for (const asn1::e1ap::up_params_item_s& ul_up_transport_param :
                 e1ap_drb_setup_item.ul_up_transport_params) {
              rrc_ue_up_params_item ue_params_item;
              ue_params_item.up_tnl_info   = asn1_to_up_transport_layer_info(ul_up_transport_param.up_tnl_info);
              ue_params_item.cell_group_id = ul_up_transport_param.cell_group_id;

              drb_setup_item.ul_up_transport_params.push_back(ue_params_item);
            }

            // Add Flow setup List
            for (auto e1ap_qos_flow_item : e1ap_drb_setup_item.flow_setup_list) {
              rrc_ue_qos_flow_item qos_flow_item;
              qos_flow_item.qos_flow_id = e1ap_qos_flow_item.qo_s_flow_id;

              drb_setup_item.flow_setup_list.push_back(qos_flow_item);
            }

            // Add Flow Failed List
            for (auto e1ap_failed_qos_flow_item : e1ap_drb_setup_item.flow_failed_list) {
              rrc_ue_qos_flow_failed_item failed_qos_flow_item;

              failed_qos_flow_item.qos_flow_id = e1ap_failed_qos_flow_item.qo_s_flow_id;
              failed_qos_flow_item.cause       = e1ap_cause_to_cu_cp_cause(e1ap_failed_qos_flow_item.cause);

              drb_setup_item.flow_failed_list.push_back(failed_qos_flow_item);
            }

            // Add DRB Data Forwarding Info Response
            if (e1ap_drb_setup_item.drb_data_forwarding_info_resp_present) {
              if (e1ap_drb_setup_item.drb_data_forwarding_info_resp.ul_data_forwarding_present) {
                drb_setup_item.drb_data_forwarding_info_resp.value().ul_data_forwarding =
                    asn1_to_up_transport_layer_info(
                        e1ap_drb_setup_item.drb_data_forwarding_info_resp.ul_data_forwarding);
              }
              if (e1ap_drb_setup_item.drb_data_forwarding_info_resp.dl_data_forwarding_present) {
                drb_setup_item.drb_data_forwarding_info_resp.value().dl_data_forwarding =
                    asn1_to_up_transport_layer_info(
                        e1ap_drb_setup_item.drb_data_forwarding_info_resp.dl_data_forwarding);
              }
            }

            res_mod_item.drb_setup_list_ng_ran.push_back(drb_setup_item);
          }

          // Add DRB session resource failed list NG RAN
          for (auto e1ap_drb_failed_item : e1ap_res_mod_item.drb_failed_mod_list_ng_ran) {
            rrc_ue_drb_failed_item_ng_ran drb_failed_item;
            drb_failed_item.drb_id = uint_to_drb_id(e1ap_drb_failed_item.drb_id);
            drb_failed_item.cause  = e1ap_cause_to_cu_cp_cause(e1ap_drb_failed_item.cause);

            res_mod_item.drb_failed_list_ng_ran.push_back(drb_failed_item);
          }

          // Add Security Result
          if (e1ap_res_mod_item.security_result_present) {
            res_mod_item.security_result.value().confidentiality_protection_result =
                e1ap_res_mod_item.security_result.confidentiality_protection_result.to_string();
            res_mod_item.security_result.value().integrity_protection_result =
                e1ap_res_mod_item.security_result.integrity_protection_result.to_string();
          }

          // Add PDU Session Data Forwarding Info Response
          if (e1ap_res_mod_item.pdu_session_data_forwarding_info_resp_present) {
            if (e1ap_res_mod_item.pdu_session_data_forwarding_info_resp.ul_data_forwarding_present) {
              res_mod_item.pdu_session_data_forwarding_info_resp.value().ul_data_forwarding =
                  asn1_to_up_transport_layer_info(
                      e1ap_res_mod_item.pdu_session_data_forwarding_info_resp.ul_data_forwarding);
            }
            if (e1ap_res_mod_item.pdu_session_data_forwarding_info_resp.dl_data_forwarding_present) {
              res_mod_item.pdu_session_data_forwarding_info_resp.value().dl_data_forwarding =
                  asn1_to_up_transport_layer_info(
                      e1ap_res_mod_item.pdu_session_data_forwarding_info_resp.dl_data_forwarding);
            }
          }

          res.pdu_session_resource_setup_list.push_back(res_mod_item);
        }
      }

      // Add PDU session resource failed list
      if (bearer_context_mod_response.pdu_session_res_failed_mod_list_present) {
        for (auto e1ap_failed_item : bearer_context_mod_response.pdu_session_res_failed_mod_list.value) {
          rrc_ue_pdu_session_resource_failed_item failed_item;

          failed_item.pdu_session_id = e1ap_failed_item.pdu_session_id;
          failed_item.cause          = e1ap_cause_to_cu_cp_cause(e1ap_failed_item.cause);

          res.pdu_session_resource_failed_list.push_back(failed_item);
        }
      }

      // Add PDU session resource modified list
      if (bearer_context_mod_response.pdu_session_res_modified_list_present) {
        for (auto e1ap_res_mod_item : bearer_context_mod_response.pdu_session_res_modified_list.value) {
          rrc_ue_pdu_session_resource_modified_item res_mod_item;

          res_mod_item.pdu_session_id = e1ap_res_mod_item.pdu_session_id;

          // Add NG DL UP TNL Info
          res_mod_item.ng_dl_up_tnl_info = asn1_to_up_transport_layer_info(e1ap_res_mod_item.ng_dl_up_tnl_info);

          // Add DRB Setup List NG RAN
          for (auto e1ap_drb_setup_item : e1ap_res_mod_item.drb_setup_list_ng_ran) {
            rrc_ue_drb_setup_item_ng_ran drb_setup_item;
            drb_setup_item.drb_id = uint_to_drb_id(e1ap_drb_setup_item.drb_id);

            // Add UL UP Transport Params
            for (const auto& ul_up_transport_param : e1ap_drb_setup_item.ul_up_transport_params) {
              rrc_ue_up_params_item ue_params_item;
              ue_params_item.up_tnl_info   = asn1_to_up_transport_layer_info(ul_up_transport_param.up_tnl_info);
              ue_params_item.cell_group_id = ul_up_transport_param.cell_group_id;

              drb_setup_item.ul_up_transport_params.push_back(ue_params_item);
            }

            // Add Flow setup List
            for (auto e1ap_qos_flow_item : e1ap_drb_setup_item.flow_setup_list) {
              rrc_ue_qos_flow_item qos_flow_item;
              qos_flow_item.qos_flow_id = e1ap_qos_flow_item.qo_s_flow_id;

              drb_setup_item.flow_setup_list.push_back(qos_flow_item);
            }

            // Add Flow Failed List
            for (auto e1ap_failed_qos_flow_item : e1ap_drb_setup_item.flow_failed_list) {
              rrc_ue_qos_flow_failed_item failed_qos_flow_item;

              failed_qos_flow_item.qos_flow_id = e1ap_failed_qos_flow_item.qo_s_flow_id;
              failed_qos_flow_item.cause       = e1ap_cause_to_cu_cp_cause(e1ap_failed_qos_flow_item.cause);

              drb_setup_item.flow_failed_list.push_back(failed_qos_flow_item);
            }

            // Add DRB Data Forwarding Info Response
            if (e1ap_drb_setup_item.drb_data_forwarding_info_resp_present) {
              if (e1ap_drb_setup_item.drb_data_forwarding_info_resp.ul_data_forwarding_present) {
                drb_setup_item.drb_data_forwarding_info_resp.value().ul_data_forwarding =
                    asn1_to_up_transport_layer_info(
                        e1ap_drb_setup_item.drb_data_forwarding_info_resp.ul_data_forwarding);
              }
              if (e1ap_drb_setup_item.drb_data_forwarding_info_resp.dl_data_forwarding_present) {
                drb_setup_item.drb_data_forwarding_info_resp.value().dl_data_forwarding =
                    asn1_to_up_transport_layer_info(
                        e1ap_drb_setup_item.drb_data_forwarding_info_resp.dl_data_forwarding);
              }
            }

            res_mod_item.drb_setup_list_ng_ran.push_back(drb_setup_item);
          }

          // Add DRB session resource failed list NG RAN
          for (auto e1ap_drb_failed_item : e1ap_res_mod_item.drb_failed_list_ng_ran) {
            rrc_ue_drb_failed_item_ng_ran drb_failed_item;
            drb_failed_item.drb_id = uint_to_drb_id(e1ap_drb_failed_item.drb_id);
            drb_failed_item.cause  = e1ap_cause_to_cu_cp_cause(e1ap_drb_failed_item.cause);

            res_mod_item.drb_failed_list_ng_ran.push_back(drb_failed_item);
          }

          // Add DRB modified list NG RAN
          for (auto e1ap_drb_mod_item : e1ap_res_mod_item.drb_modified_list_ng_ran) {
            rrc_ue_drb_modified_item_ng_ran drb_mod_item;
            drb_mod_item.drb_id = uint_to_drb_id(e1ap_drb_mod_item.drb_id);

            // Add UL UP Transport Params
            for (const auto& ul_up_transport_param : e1ap_drb_mod_item.ul_up_transport_params) {
              rrc_ue_up_params_item ue_params_item;
              ue_params_item.up_tnl_info   = asn1_to_up_transport_layer_info(ul_up_transport_param.up_tnl_info);
              ue_params_item.cell_group_id = ul_up_transport_param.cell_group_id;

              drb_mod_item.ul_up_transport_params.push_back(ue_params_item);
            }

            // Add Flow setup List
            for (auto e1ap_qos_flow_item : e1ap_drb_mod_item.flow_setup_list) {
              rrc_ue_qos_flow_item qos_flow_item;
              qos_flow_item.qos_flow_id = e1ap_qos_flow_item.qo_s_flow_id;

              drb_mod_item.flow_setup_list.push_back(qos_flow_item);
            }

            // Add Flow Failed List
            for (auto e1ap_failed_qos_flow_item : e1ap_drb_mod_item.flow_failed_list) {
              rrc_ue_qos_flow_failed_item failed_qos_flow_item;

              failed_qos_flow_item.qos_flow_id = e1ap_failed_qos_flow_item.qo_s_flow_id;
              failed_qos_flow_item.cause       = e1ap_cause_to_cu_cp_cause(e1ap_failed_qos_flow_item.cause);

              drb_mod_item.flow_failed_list.push_back(failed_qos_flow_item);
            }

            // Add PDCP SN status info
            if (e1ap_drb_mod_item.pdcp_sn_status_info_present) {
              auto& e1ap_pdcp_sn_status_info = e1ap_drb_mod_item.pdcp_sn_status_info;

              drb_mod_item.pdcp_sn_status_info.value().pdcp_status_transfer_ul.count_value =
                  e1ap_asn1_pdcp_count_to_pdcp_count(e1ap_pdcp_sn_status_info.pdcp_status_transfer_ul.count_value);

              if (e1ap_pdcp_sn_status_info.pdcp_status_transfer_ul.receive_statusof_pdcpsdu_present) {
                drb_mod_item.pdcp_sn_status_info.value().pdcp_status_transfer_ul.receive_statusof_pdcpsdu =
                    e1ap_pdcp_sn_status_info.pdcp_status_transfer_ul.receive_statusof_pdcpsdu.to_number();
              }

              drb_mod_item.pdcp_sn_status_info.value().pdcp_status_transfer_dl =
                  e1ap_asn1_pdcp_count_to_pdcp_count(e1ap_pdcp_sn_status_info.pdcp_status_transfer_dl);
            }

            res_mod_item.drb_modified_list_ng_ran.push_back(drb_mod_item);
          }

          // Add DRB failed to modify list NG RAN
          for (auto e1ap_drb_failed_item : e1ap_res_mod_item.drb_failed_to_modify_list_ng_ran) {
            rrc_ue_drb_failed_item_ng_ran drb_failed_item;
            drb_failed_item.drb_id = uint_to_drb_id(e1ap_drb_failed_item.drb_id);
            drb_failed_item.cause  = e1ap_cause_to_cu_cp_cause(e1ap_drb_failed_item.cause);

            res_mod_item.drb_failed_to_modify_list_ng_ran.push_back(drb_failed_item);
          }

          // Add Security Result
          if (e1ap_res_mod_item.security_result_present) {
            res_mod_item.security_result.value().confidentiality_protection_result =
                e1ap_res_mod_item.security_result.confidentiality_protection_result.to_string();
            res_mod_item.security_result.value().integrity_protection_result =
                e1ap_res_mod_item.security_result.integrity_protection_result.to_string();
          }

          // Add PDU Session Data Forwarding Info Response
          if (e1ap_res_mod_item.pdu_session_data_forwarding_info_resp_present) {
            if (e1ap_res_mod_item.pdu_session_data_forwarding_info_resp.ul_data_forwarding_present) {
              res_mod_item.pdu_session_data_forwarding_info_resp.value().ul_data_forwarding =
                  asn1_to_up_transport_layer_info(
                      e1ap_res_mod_item.pdu_session_data_forwarding_info_resp.ul_data_forwarding);
            }
            if (e1ap_res_mod_item.pdu_session_data_forwarding_info_resp.dl_data_forwarding_present) {
              res_mod_item.pdu_session_data_forwarding_info_resp.value().dl_data_forwarding =
                  asn1_to_up_transport_layer_info(
                      e1ap_res_mod_item.pdu_session_data_forwarding_info_resp.dl_data_forwarding);
            }
          }

          res.pdu_session_resource_modified_list.push_back(res_mod_item);
        }
      }

      // Add PDU session resource failed to modify list
      if (bearer_context_mod_response.pdu_session_res_failed_to_modify_list_present) {
        for (auto e1ap_failed_item : bearer_context_mod_response.pdu_session_res_failed_to_modify_list.value) {
          rrc_ue_pdu_session_resource_failed_item failed_item;

          failed_item.pdu_session_id = e1ap_failed_item.pdu_session_id;
          failed_item.cause          = e1ap_cause_to_cu_cp_cause(e1ap_failed_item.cause);

          res.pdu_session_resource_failed_to_modify_list.push_back(failed_item);
        }
      }
    }
  } else {
    res.success = false;
    res.cause   = e1ap_cause_to_cu_cp_cause(e1_bearer_context_modification_resp.failure->cause.value);
    if (e1_bearer_context_modification_resp.failure->crit_diagnostics_present) {
      // TODO: Add crit diagnostics
    }
  }
}

} // namespace srs_cu_cp
} // namespace srsgnb
