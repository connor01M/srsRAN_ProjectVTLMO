/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "demodulation_mapper_impl.h"
#include "srsgnb/support/math_utils.h"
#include "srsgnb/support/srsgnb_assert.h"

using namespace srsgnb;

// Square root of 1/10.
static const float M_SQRT1_10 = 1.0F / std::sqrt(10.0F);

// Square root of 1/42.
static const float M_SQRT1_42 = 1.0F / std::sqrt(42.0F);

// Square root of 1/170.
static const float M_SQRT1_170 = 1.0F / std::sqrt(170.0F);

static log_likelihood_ratio demod_BPSK_symbol(cf_t z, float noise_var, float range_limit)
{
  float l_value = 2.0F * M_SQRT2f32 * (std::real(z) + std::imag(z)) / noise_var;
  return log_likelihood_ratio::quantize(l_value, range_limit);
};

static void
demodulate_soft_BPSK(span<log_likelihood_ratio> llrs, span<const cf_t> symbols, span<const float> noise_vars)
{
  // Maximum (absolute) value considered for quantization. Larger values will be clipped.
  constexpr float RANGE_LIMIT_FLOAT = 600;
  std::transform(symbols.begin(), symbols.end(), noise_vars.begin(), llrs.begin(), [](cf_t z, float n) {
    return demod_BPSK_symbol(z, n, RANGE_LIMIT_FLOAT);
  });
}

static log_likelihood_ratio demod_QPSK_symbol(float x, float noise_var, float range_limit)
{
  float l_value = 2.0F * M_SQRT2f32 * x / noise_var;
  return log_likelihood_ratio::quantize(l_value, range_limit);
};

static void
demodulate_soft_QPSK(span<log_likelihood_ratio> llrs, span<const cf_t> symbols, span<const float> noise_vars)
{
  // Maximum (absolute) value considered for quantization. Larger values will be clipped.
  constexpr float RANGE_LIMIT_FLOAT = 600;

  auto*       llr_it   = llrs.begin();
  const auto* noise_it = noise_vars.begin();
  for (const auto this_symbol : symbols) {
    *llr_it++ = demod_QPSK_symbol(std::real(this_symbol), *noise_it, RANGE_LIMIT_FLOAT);
    *llr_it++ = demod_QPSK_symbol(std::imag(this_symbol), *noise_it, RANGE_LIMIT_FLOAT);
    ++noise_it;
  }
}

static log_likelihood_ratio demod_16QAM_symbol_01(float x, float noise_var, float range_limit)
{
  float l_value = 4 * M_SQRT1_10 * x;
  l_value       = (std::abs(x) <= 2 * M_SQRT1_10) ? l_value : 2 * l_value - std::copysign(0.8F, x);
  l_value /= noise_var;
  return log_likelihood_ratio::quantize(l_value, range_limit);
};

static log_likelihood_ratio demod_16QAM_symbol_23(float x, float noise_var, float range_limit)
{
  float l_value = 0.8F - 4 * M_SQRT1_10 * std::abs(x);
  l_value /= noise_var;
  return log_likelihood_ratio::quantize(l_value, range_limit);
};

static void
demodulate_soft_QAM16(span<log_likelihood_ratio> llrs, span<const cf_t> symbols, span<const float> noise_vars)
{
  // Maximum (absolute) value considered for quantization. Larger values will be clipped.
  constexpr float RANGE_LIMIT_FLOAT = 300;

  auto*       llr_it   = llrs.begin();
  const auto* noise_it = noise_vars.begin();
  for (const auto this_symbol : symbols) {
    *llr_it++ = demod_16QAM_symbol_01(std::real(this_symbol), *noise_it, RANGE_LIMIT_FLOAT);
    *llr_it++ = demod_16QAM_symbol_01(std::imag(this_symbol), *noise_it, RANGE_LIMIT_FLOAT);
    *llr_it++ = demod_16QAM_symbol_23(std::real(this_symbol), *noise_it, RANGE_LIMIT_FLOAT);
    *llr_it++ = demod_16QAM_symbol_23(std::imag(this_symbol), *noise_it, RANGE_LIMIT_FLOAT);
    ++noise_it;
  }
}

unsigned compute_interval_idx(float x, float interval_width, unsigned nof_intervals)
{
  int nof_intervals_int = static_cast<int>(nof_intervals);
  int idx               = static_cast<int>(std::floor(x / interval_width)) + nof_intervals_int / 2;

  return clamp(idx, 0, nof_intervals_int - 1);
}

static log_likelihood_ratio demod_64QAM_symbol_01(float x, float noise_var, float range_limit)
{
  constexpr unsigned                            NOF_INTERVALS = 8;
  static const std::array<float, NOF_INTERVALS> SLOPE         = {16 * M_SQRT1_42,
                                                                 12 * M_SQRT1_42,
                                                                 8 * M_SQRT1_42,
                                                                 4 * M_SQRT1_42,
                                                                 4 * M_SQRT1_42,
                                                                 8 * M_SQRT1_42,
                                                                 12 * M_SQRT1_42,
                                                                 16 * M_SQRT1_42};
  static const std::array<float, NOF_INTERVALS> INTERCEPT     = {
          24.0F / 21, 12.0F / 21, 4.0F / 21, 0.0F, 0.0F, -4.0F / 21, -12.0F / 21, -24.0F / 21};

  unsigned idx     = compute_interval_idx(x, 2 * M_SQRT1_42, NOF_INTERVALS);
  float    l_value = SLOPE[idx] * x + INTERCEPT[idx];
  l_value /= noise_var;
  return log_likelihood_ratio::quantize(l_value, range_limit);
}

static log_likelihood_ratio demod_64QAM_symbol_23(float x, float noise_var, float range_limit)
{
  constexpr unsigned                            NOF_INTERVALS = 8;
  static const std::array<float, NOF_INTERVALS> SLOPE         = {8 * M_SQRT1_42,
                                                                 4 * M_SQRT1_42,
                                                                 4 * M_SQRT1_42,
                                                                 8 * M_SQRT1_42,
                                                                 -8 * M_SQRT1_42,
                                                                 -4 * M_SQRT1_42,
                                                                 -4 * M_SQRT1_42,
                                                                 -8 * M_SQRT1_42};
  static const std::array<float, NOF_INTERVALS> INTERCEPT     = {
          20.0F / 21, 8.0F / 21, 8.0F / 21, 12.0F / 21, 12.0F / 21, 8.0F / 21, 8.0F / 21, 20.0F / 21};

  unsigned idx     = compute_interval_idx(x, 2 * M_SQRT1_42, NOF_INTERVALS);
  float    l_value = SLOPE[idx] * x + INTERCEPT[idx];
  l_value /= noise_var;
  return log_likelihood_ratio::quantize(l_value, range_limit);
}

static log_likelihood_ratio demod_64QAM_symbol_45(float x, float noise_var, float range_limit)
{
  constexpr unsigned                            NOF_INTERVALS = 4;
  static const std::array<float, NOF_INTERVALS> SLOPE         = {
              4 * M_SQRT1_42, -4 * M_SQRT1_42, 4 * M_SQRT1_42, -4 * M_SQRT1_42};
  static const std::array<float, NOF_INTERVALS> INTERCEPT = {12.0F / 21, -4.0F / 21, -4.0F / 21, 12.0F / 21};

  unsigned idx     = compute_interval_idx(x, 4 * M_SQRT1_42, NOF_INTERVALS);
  float    l_value = SLOPE[idx] * x + INTERCEPT[idx];
  l_value /= noise_var;
  return log_likelihood_ratio::quantize(l_value, range_limit);
}

static void
demodulate_soft_QAM64(span<log_likelihood_ratio> llrs, span<const cf_t> symbols, span<const float> noise_vars)
{
  // Maximum (absolute) value considered for quantization. Larger values will be clipped.
  constexpr float RANGE_LIMIT_FLOAT = 150;

  auto*       llr_it   = llrs.begin();
  const auto* noise_it = noise_vars.begin();
  for (const auto this_symbol : symbols) {
    *llr_it++ = demod_64QAM_symbol_01(std::real(this_symbol), *noise_it, RANGE_LIMIT_FLOAT);
    *llr_it++ = demod_64QAM_symbol_01(std::imag(this_symbol), *noise_it, RANGE_LIMIT_FLOAT);
    *llr_it++ = demod_64QAM_symbol_23(std::real(this_symbol), *noise_it, RANGE_LIMIT_FLOAT);
    *llr_it++ = demod_64QAM_symbol_23(std::imag(this_symbol), *noise_it, RANGE_LIMIT_FLOAT);
    *llr_it++ = demod_64QAM_symbol_45(std::real(this_symbol), *noise_it, RANGE_LIMIT_FLOAT);
    *llr_it++ = demod_64QAM_symbol_45(std::imag(this_symbol), *noise_it, RANGE_LIMIT_FLOAT);
    ++noise_it;
  }
}

static log_likelihood_ratio demod_256QAM_symbol_01(float x, float noise_var, float range_limit)
{
  constexpr unsigned                            NOF_INTERVALS = 16;
  static const std::array<float, NOF_INTERVALS> SLOPE         = {32 * M_SQRT1_170,
                                                                 28 * M_SQRT1_170,
                                                                 24 * M_SQRT1_170,
                                                                 20 * M_SQRT1_170,
                                                                 16 * M_SQRT1_170,
                                                                 12 * M_SQRT1_170,
                                                                 8 * M_SQRT1_170,
                                                                 4 * M_SQRT1_170,
                                                                 4 * M_SQRT1_170,
                                                                 8 * M_SQRT1_170,
                                                                 12 * M_SQRT1_170,
                                                                 16 * M_SQRT1_170,
                                                                 20 * M_SQRT1_170,
                                                                 24 * M_SQRT1_170,
                                                                 28 * M_SQRT1_170,
                                                                 32 * M_SQRT1_170};
  static const std::array<float, NOF_INTERVALS> INTERCEPT     = {112.0F / 85,
                                                                 84.0F / 85,
                                                                 60.0F / 85,
                                                                 40.0F / 85,
                                                                 24.0F / 85,
                                                                 12.0F / 85,
                                                                 4.0F / 85,
                                                                 0.0F,
                                                                 0.0F,
                                                                 -4.0F / 85,
                                                                 -12.0F / 85,
                                                                 -24.0F / 85,
                                                                 -40.0F / 85,
                                                                 -60.0F / 85,
                                                                 -84.0F / 85,
                                                                 -112.0F / 85};

  unsigned idx     = compute_interval_idx(x, 2 * M_SQRT1_170, NOF_INTERVALS);
  float    l_value = SLOPE[idx] * x + INTERCEPT[idx];
  l_value /= noise_var;
  return log_likelihood_ratio::quantize(l_value, range_limit);
}

static log_likelihood_ratio demod_256QAM_symbol_23(float x, float noise_var, float range_limit)
{
  constexpr unsigned                            NOF_INTERVALS = 16;
  static const std::array<float, NOF_INTERVALS> SLOPE         = {16 * M_SQRT1_170,
                                                                 12 * M_SQRT1_170,
                                                                 8 * M_SQRT1_170,
                                                                 4 * M_SQRT1_170,
                                                                 4 * M_SQRT1_170,
                                                                 8 * M_SQRT1_170,
                                                                 12 * M_SQRT1_170,
                                                                 16 * M_SQRT1_170,
                                                                 -16 * M_SQRT1_170,
                                                                 -12 * M_SQRT1_170,
                                                                 -8 * M_SQRT1_170,
                                                                 -4 * M_SQRT1_170,
                                                                 -4 * M_SQRT1_170,
                                                                 -8 * M_SQRT1_170,
                                                                 -12 * M_SQRT1_170,
                                                                 -16 * M_SQRT1_170};
  static const std::array<float, NOF_INTERVALS> INTERCEPT     = {88.0F / 85,
                                                                 60.0F / 85,
                                                                 36.0F / 85,
                                                                 16.0F / 85,
                                                                 16.0F / 85,
                                                                 28.0F / 85,
                                                                 36.0F / 85,
                                                                 40.0F / 85,
                                                                 40.0F / 85,
                                                                 36.0F / 85,
                                                                 28.0F / 85,
                                                                 16.0F / 85,
                                                                 16.0F / 85,
                                                                 36.0F / 85,
                                                                 60.0F / 85,
                                                                 88.0F / 85};

  unsigned idx     = compute_interval_idx(x, 2 * M_SQRT1_170, NOF_INTERVALS);
  float    l_value = SLOPE[idx] * x + INTERCEPT[idx];
  l_value /= noise_var;
  return log_likelihood_ratio::quantize(l_value, range_limit);
}

static log_likelihood_ratio demod_256QAM_symbol_45(float x, float noise_var, float range_limit)
{
  constexpr unsigned                            NOF_INTERVALS = 16;
  static const std::array<float, NOF_INTERVALS> SLOPE         = {8 * M_SQRT1_170,
                                                                 4 * M_SQRT1_170,
                                                                 4 * M_SQRT1_170,
                                                                 8 * M_SQRT1_170,
                                                                 -8 * M_SQRT1_170,
                                                                 -4 * M_SQRT1_170,
                                                                 -4 * M_SQRT1_170,
                                                                 -8 * M_SQRT1_170,
                                                                 8 * M_SQRT1_170,
                                                                 4 * M_SQRT1_170,
                                                                 4 * M_SQRT1_170,
                                                                 8 * M_SQRT1_170,
                                                                 -8 * M_SQRT1_170,
                                                                 -4 * M_SQRT1_170,
                                                                 -4 * M_SQRT1_170,
                                                                 -8 * M_SQRT1_170};
  static const std::array<float, NOF_INTERVALS> INTERCEPT     = {52.0F / 85,
                                                                 24.0F / 85,
                                                                 24.0F / 85,
                                                                 44.0F / 85,
                                                                 -20.0F / 85,
                                                                 -8.0F / 85,
                                                                 -8.0F / 85,
                                                                 -12.0F / 85,
                                                                 -12.0F / 85,
                                                                 -8.0F / 85,
                                                                 -8.0F / 85,
                                                                 -20.0F / 85,
                                                                 44.0F / 85,
                                                                 24.0F / 85,
                                                                 24.0F / 85,
                                                                 52.0F / 85};

  unsigned idx     = compute_interval_idx(x, 2 * M_SQRT1_170, NOF_INTERVALS);
  float    l_value = SLOPE[idx] * x + INTERCEPT[idx];
  l_value /= noise_var;
  return log_likelihood_ratio::quantize(l_value, range_limit);
}

static log_likelihood_ratio demod_256QAM_symbol_67(float x, float noise_var, float range_limit)
{
  constexpr unsigned                            NOF_INTERVALS = 8;
  static const std::array<float, NOF_INTERVALS> SLOPE         = {4 * M_SQRT1_170,
                                                                 -4 * M_SQRT1_170,
                                                                 4 * M_SQRT1_170,
                                                                 -4 * M_SQRT1_170,
                                                                 4 * M_SQRT1_170,
                                                                 -4 * M_SQRT1_170,
                                                                 4 * M_SQRT1_170,
                                                                 -4 * M_SQRT1_170};
  static const std::array<float, NOF_INTERVALS> INTERCEPT     = {
          28.0F / 85, -20.0F / 85, 12.0F / 85, -4.0F / 85, -4.0F / 85, 12.0F / 85, -20.0F / 85, 28.0F / 85};

  unsigned idx     = compute_interval_idx(x, 4 * M_SQRT1_170, NOF_INTERVALS);
  float    l_value = SLOPE[idx] * x + INTERCEPT[idx];
  l_value /= noise_var;
  return log_likelihood_ratio::quantize(l_value, range_limit);
}

static void
demodulate_soft_QAM256(span<log_likelihood_ratio> llrs, span<const cf_t> symbols, span<const float> noise_vars)
{
  // Maximum (absolute) value considered for quantization. Larger values will be clipped.
  constexpr float RANGE_LIMIT_FLOAT = 90;

  auto*       llr_it   = llrs.begin();
  const auto* noise_it = noise_vars.begin();
  for (const auto this_symbol : symbols) {
    *llr_it++ = demod_256QAM_symbol_01(std::real(this_symbol), *noise_it, RANGE_LIMIT_FLOAT);
    *llr_it++ = demod_256QAM_symbol_01(std::imag(this_symbol), *noise_it, RANGE_LIMIT_FLOAT);
    *llr_it++ = demod_256QAM_symbol_23(std::real(this_symbol), *noise_it, RANGE_LIMIT_FLOAT);
    *llr_it++ = demod_256QAM_symbol_23(std::imag(this_symbol), *noise_it, RANGE_LIMIT_FLOAT);
    *llr_it++ = demod_256QAM_symbol_45(std::real(this_symbol), *noise_it, RANGE_LIMIT_FLOAT);
    *llr_it++ = demod_256QAM_symbol_45(std::imag(this_symbol), *noise_it, RANGE_LIMIT_FLOAT);
    *llr_it++ = demod_256QAM_symbol_67(std::real(this_symbol), *noise_it, RANGE_LIMIT_FLOAT);
    *llr_it++ = demod_256QAM_symbol_67(std::imag(this_symbol), *noise_it, RANGE_LIMIT_FLOAT);
    ++noise_it;
  }
}

void demodulation_mapper_impl::demodulate_soft(span<log_likelihood_ratio> llrs,
                                               span<const cf_t>           symbols,
                                               span<const float>          noise_vars,
                                               modulation_scheme          mod)
{
  srsgnb_assert(symbols.size() == noise_vars.size(), "Inputs symbols and noise_vars must have the same length.");
  srsgnb_assert(symbols.size() * get_bits_per_symbol(mod) == llrs.size(), "Input and output lengths are incompatible.");
  srsgnb_assert(std::all_of(noise_vars.begin(), noise_vars.end(), [](float f) { return f > 0; }),
                "Input noise_vars must have positive entries.");

  switch (mod) {
    case modulation_scheme::BPSK:
      demodulate_soft_BPSK(llrs, symbols, noise_vars);
      break;
    case modulation_scheme::QPSK:
      demodulate_soft_QPSK(llrs, symbols, noise_vars);
      break;
    case modulation_scheme::QAM16:
      demodulate_soft_QAM16(llrs, symbols, noise_vars);
      break;
    case modulation_scheme::QAM64:
      demodulate_soft_QAM64(llrs, symbols, noise_vars);
      break;
    case modulation_scheme::QAM256:
      demodulate_soft_QAM256(llrs, symbols, noise_vars);
      break;
    default:
      assert(false);
  }
}
