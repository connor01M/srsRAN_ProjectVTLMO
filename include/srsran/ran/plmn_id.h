/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsran/adt/expected.h"
#include "srsran/adt/static_vector.h"
#include "srsran/ran/bcd_helpers.h"
#include <array>
#include <cstdint>
#include <string>

namespace srsran {

/// Identifier of a Mobile Country Code (MCC).
class mobile_country_code
{
  mobile_country_code(uint16_t mcc) { data = mcc; }

public:
  mobile_country_code() = delete;

  /// Conversion of a BCD-encoded MCC integer into a mobile_country_code.
  static expected<mobile_country_code> from_bcd(uint16_t bcd_mcc)
  {
    if (!bcd_helper::is_valid_mcc(bcd_mcc)) {
      return default_error_t{};
    }
    return mobile_country_code{bcd_mcc};
  }

  /// Conversion of string of 3 digits into a mobile_country_code.
  static expected<mobile_country_code> from_string(const std::string& mcc_str)
  {
    uint16_t mcc;
    if (!string_to_mcc(mcc_str, &mcc)) {
      return default_error_t{};
    }
    return mobile_country_code{mcc};
  }

  uint16_t to_bcd() const { return data; }

  std::string to_string() const
  {
    std::string str;
    mcc_to_string(data, &str);
    return str;
  }

  std::array<uint8_t, 3> to_bytes() const
  {
    std::array<uint8_t, 3> bytes = {};
    mcc_to_bytes(data, bytes.data());
    return bytes;
  }

  static size_t nof_digits() { return 3; }

  bool operator==(const mobile_country_code& rhs) const { return data == rhs.data; }
  bool operator!=(const mobile_country_code& rhs) const { return data != rhs.data; }

private:
  uint16_t data;
};

/// Identifier of a Mobile Network Code (MNC).
class mobile_network_code
{
  mobile_network_code(uint16_t mnc) { data = mnc; }

public:
  mobile_network_code() = delete;

  /// Conversion of a BCD-encoded MNC integer into a mobile_network_code.
  static expected<mobile_network_code> from_bcd(uint16_t bcd_mnc)
  {
    if (!bcd_helper::is_valid_mnc(bcd_mnc)) {
      return default_error_t{};
    }
    return mobile_network_code{bcd_mnc};
  }

  /// Conversion of string of 2/3 digits into a mobile_network_code.
  static expected<mobile_network_code> from_string(const std::string& mnc_str)
  {
    uint16_t mnc;
    if (!string_to_mnc(mnc_str, &mnc)) {
      return default_error_t{};
    }
    return mobile_network_code{mnc};
  }

  uint16_t to_bcd() const { return data; }

  std::string to_string() const
  {
    std::string str;
    mnc_to_string(data, &str);
    return str;
  }

  static_vector<uint8_t, 3> to_bytes() const
  {
    static_vector<uint8_t, 3> bytes(3);
    uint8_t                   len;
    mnc_to_bytes(data, bytes.data(), &len);
    bytes.resize(len);
    return bytes;
  }

  /// Number of digits of the MNC.
  size_t nof_digits() const { return (data & 0xf00U) == 0xf00U ? 2 : 3; }

  bool operator==(const mobile_network_code& rhs) const { return data == rhs.data; }
  bool operator!=(const mobile_network_code& rhs) const { return data != rhs.data; }

private:
  uint16_t data;
};

/// Identifier of a Public Land Mobile Network (PLMN).
class plmn_identity
{
  plmn_identity(uint32_t bcd_plmn) : data(bcd_plmn) {}
  plmn_identity(uint16_t bcd_mcc, uint16_t bcd_mnc) { ngap_mccmnc_to_plmn(bcd_mcc, bcd_mnc, &data); }

public:
  plmn_identity() = delete;
  plmn_identity(mobile_country_code mcc, mobile_network_code mnc) : plmn_identity(mcc.to_bcd(), mnc.to_bcd()) {}

  static expected<plmn_identity> from_string(const std::string& plmn_id_str)
  {
    uint32_t plmn = plmn_string_to_bcd(plmn_id_str);
    if (plmn == 0) {
      return default_error_t{};
    }
    return plmn_identity{plmn};
  }

  uint32_t to_bcd() const { return data; }

  mobile_country_code mcc() const
  {
    uint16_t mcc, dummy;
    ngap_plmn_to_mccmnc(data, &mcc, &dummy);
    return mobile_country_code::from_bcd(mcc).value();
  }

  mobile_network_code mnc() const
  {
    uint16_t dummy, mnc;
    ngap_plmn_to_mccmnc(data, &dummy, &mnc);
    return mobile_network_code::from_bcd(mnc).value();
  }

  std::string to_string() const { return plmn_bcd_to_string(data); }

  bool operator==(const plmn_identity& rhs) const { return data == rhs.data; }
  bool operator!=(const plmn_identity& rhs) const { return data != rhs.data; }

private:
  uint32_t data;
};

} // namespace srsran

namespace fmt {

template <>
struct formatter<srsran::mobile_country_code> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const srsran::mobile_country_code& val, FormatContext& ctx)
  {
    return format_to(ctx.out(), "{}", val.to_string());
  }
};

template <>
struct formatter<srsran::mobile_network_code> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const srsran::mobile_network_code& val, FormatContext& ctx)
  {
    return format_to(ctx.out(), "{}", val.to_string());
  }
};

template <>
struct formatter<srsran::plmn_identity> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const srsran::plmn_identity& val, FormatContext& ctx)
  {
    return format_to(ctx.out(), "{}", val.to_string());
  }
};

} // namespace fmt