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

#include "srsgnb/adt/bounded_bitset.h"
#include "srsgnb/adt/optional.h"
#include "srsgnb/adt/variant.h"

namespace srsgnb {

/// \brief CodebookConfig is used to configure codebooks of Type-I and Type-II.
/// \remark TS 38.331, \c CodebookConfig and TS 38.214, clause 5.2.2.2.
struct codebook_config {
  struct type1 {
    /// \brief See TS 38.331, \c typeI-SinglePanel in \c CodebookConfig.
    struct single_panel {
      /// \brief See TS 38.331, \c nrOfAntennaPorts in \c CodebookConfig.
      using two_antenna_ports_two_tx_codebook_subset_restriction = bounded_bitset<6>;

      /// \brief See TS 38.331, \c nrOfAntennaPorts in \c CodebookConfig.
      struct more_than_two_antenna_ports {
        /// \brief See TS 38.331, \c n1-n2 in \c CodebookConfig.
        /// \remark Bitset size is based on \c n1_n2_type_t with maximum bit string length.
        using n1_n2_value_t = bounded_bitset<256>;
        enum class n1_n2_type_t {
          two_one_typei_single_panel_restriction,
          two_two_typei_single_panel_restriction,
          four_one_typei_single_panel_restriction,
          three_two_typei_single_panel_restriction,
          six_one_typei_single_panel_restriction,
          four_two_typei_single_panel_restriction,
          eight_one_typei_single_panel_restriction,
          four_three_typei_single_panel_restriction,
          six_two_typei_single_panel_restriction,
          twelve_one_typei_single_panel_restriction,
          four_four_typei_single_panel_restriction,
          eight_two_typei_single_panel_restriction,
          sixteen_one_typei_single_panel_restriction
        };

        n1_n2_type_t  n1_n2_type;
        n1_n2_value_t n1_n2_value;
        /// i2 codebook subset restriction for Type I Single-panel codebook used when reportQuantity is CRI/Ri/i1/CQI.
        optional<bounded_bitset<16>> typei_single_panel_codebook_subset_restriction_i2;

        bool operator==(const more_than_two_antenna_ports& rhs) const
        {
          return n1_n2_type == rhs.n1_n2_type && n1_n2_value == rhs.n1_n2_value &&
                 typei_single_panel_codebook_subset_restriction_i2 ==
                     rhs.typei_single_panel_codebook_subset_restriction_i2;
        }
        bool operator!=(const more_than_two_antenna_ports& rhs) const { return !(rhs == *this); }
      };

      variant<two_antenna_ports_two_tx_codebook_subset_restriction, more_than_two_antenna_ports> nof_antenna_ports;
      /// Restriction for RI for typeI-SinglePanel-RI-Restriction.
      bounded_bitset<8> typei_single_panel_ri_restriction;

      bool operator==(const single_panel& rhs) const
      {
        return nof_antenna_ports == rhs.nof_antenna_ports &&
               typei_single_panel_ri_restriction == rhs.typei_single_panel_ri_restriction;
      }
      bool operator!=(const single_panel& rhs) const { return !(rhs == *this); }
    };

    /// \brief See TS 38.331, \c typeI-MultiPanel in \c CodebookConfig.
    struct multi_panel {
      /// \brief See TS 38.331, \c ng-n1-n2 in \c CodebookConfig.
      /// \remark Bitset size is based on \c ng_n1_n2_type_t with maximum bit string length.
      using ng_n1_n2_value_t = bounded_bitset<128>;
      enum class ng_n1_n2_type_t {
        two_two_one_typei_multi_panel_restriction,
        two_four_one_typei_multi_panel_restriction,
        four_two_one_typei_multi_panel_restriction,
        two_two_two_typei_multi_panel_restriction,
        two_eight_one_typei_multi_panel_restriction,
        four_four_one_typei_multi_panel_restriction,
        two_four_two_typei_multi_panel_restriction,
        four_two_two_typei_multi_panel_restriction
      };

      ng_n1_n2_type_t  ng_n1_n2_type;
      ng_n1_n2_value_t ng_n1_n2_value;
      /// Restriction for RI for typeI-MultiPanel-RI-Restriction.
      bounded_bitset<4> ri_restriction;

      bool operator==(const multi_panel& rhs) const
      {
        return ng_n1_n2_type == rhs.ng_n1_n2_type && ng_n1_n2_value == rhs.ng_n1_n2_value &&
               ri_restriction == rhs.ri_restriction;
      }
      bool operator!=(const multi_panel& rhs) const { return !(rhs == *this); }
    };

    variant<single_panel, multi_panel> sub_type;
    /// CodebookMode as specified in TS 38.214, clause 5.2.2.2.2. Value {1,...,2}.
    unsigned codebook_mode;

    bool operator==(const type1& rhs) const { return sub_type == rhs.sub_type && codebook_mode == rhs.codebook_mode; }
    bool operator!=(const type1& rhs) const { return !(rhs == *this); }
  };

  struct type2 {
    /// \brief See TS 38.331, \c typeII in \c CodebookConfig.
    struct typeii {
      /// \brief See TS 38.331, \c n1-n2-codebookSubsetRestriction in \c CodebookConfig.
      /// \remark Bitset size is based on \c n1_n2_codebook_subset_restriction_type_t with maximum bit string length.
      using n1_n2_codebook_subset_restriction_value_t = bounded_bitset<139>;
      enum class n1_n2_codebook_subset_restriction_type_t {
        two_one,
        two_two,
        four_one,
        three_two,
        six_one,
        four_two,
        eight_one,
        four_three,
        six_two,
        twelve_one,
        four_four,
        eight_two,
        sixteen_one
      };

      n1_n2_codebook_subset_restriction_type_t  n1_n2_codebook_subset_restriction_type;
      n1_n2_codebook_subset_restriction_value_t n1_n2_codebook_subset_restriction_value;
      /// Restriction for RI for TypeII-RI-Restriction.
      bounded_bitset<2> typeii_ri_restriction;

      bool operator==(const typeii& rhs) const
      {
        return n1_n2_codebook_subset_restriction_type == rhs.n1_n2_codebook_subset_restriction_type &&
               n1_n2_codebook_subset_restriction_value == rhs.n1_n2_codebook_subset_restriction_value &&
               typeii_ri_restriction == rhs.typeii_ri_restriction;
      }
      bool operator!=(const typeii& rhs) const { return !(rhs == *this); }
    };

    /// \brief See TS 38.331, \c typeII-PortSelection in \c CodebookConfig.
    struct typeii_port_selection {
      /// The size of the port selection codebook (parameter d). See TS 38.214 clause 5.2.2.2.6. Values {1, 2, 3, 4}.
      optional<unsigned> port_selection_sampling_size;
      /// Restriction for RI for TypeII-PortSelection-RI-Restriction.
      bounded_bitset<2> typeii_port_selection_ri_restriction;

      bool operator==(const typeii_port_selection& rhs) const
      {
        return port_selection_sampling_size == rhs.port_selection_sampling_size &&
               typeii_port_selection_ri_restriction == rhs.typeii_port_selection_ri_restriction;
      }
      bool operator!=(const typeii_port_selection& rhs) const { return !(rhs == *this); }
    };

    variant<typeii, typeii_port_selection> sub_type;
    /// The size of the PSK alphabet, QPSK or 8-PSK. Values {4, 8}.
    unsigned phase_alphabet_size;
    /// If subband amplitude reporting is activated (true).
    bool subband_amplitude;
    /// Number of beams, L, used for linear combination. Values {2, 3, 4}.
    unsigned nof_beams;

    bool operator==(const type2& rhs) const
    {
      return sub_type == rhs.sub_type && phase_alphabet_size == rhs.phase_alphabet_size &&
             subband_amplitude == rhs.subband_amplitude && nof_beams == rhs.nof_beams;
    }
    bool operator!=(const type2& rhs) const { return !(rhs == *this); }
  };

  variant<type1, type2> codebook_type;

  bool operator==(const codebook_config& rhs) const { return codebook_type == rhs.codebook_type; }
  bool operator!=(const codebook_config& rhs) const { return !(rhs == *this); }
};

} // namespace srsgnb