#pragma once

#include <complex>

#include "buffer/buffer.h"

namespace tvsc::transform {

template <size_t IN_BUFFER_SIZE, size_t OUT_BUFFER_SIZE = IN_BUFFER_SIZE>
void psm_s16le_to_iq(const buffer::Buffer<short, IN_BUFFER_SIZE>& in,
                     buffer::Buffer<std::complex<float>, OUT_BUFFER_SIZE>& out) {
  psm_s16le_to_iq(in, OUT_BUFFER_SIZE, out.data());
}

template <size_t BUFFER_SIZE>
void psm_s16le_to_iq(const buffer::Buffer<short, BUFFER_SIZE>& in, size_t out_size, std::complex<float>* out) {
  // TODO(james): Hack. Use a proper Hilbert transform.
  for (size_t i = 0; i < std::min(BUFFER_SIZE, out_size); ++i) {
    // TODO(james): Determine why this conversion is narrowing.
    float value{in[i]};
    std::complex<float> result{value, 0.f};
    out[i] = result;
  }
}

}  // namespace tvsc::transform
