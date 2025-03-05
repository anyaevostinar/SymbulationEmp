#pragma once

#include "emp/base/array.hpp"

#include <algorithm>

namespace sgpmode {

// TODO - write tests for IORingBuffer
/// A helper class for a ring buffer that keeps the latest `len` inputs and
/// discards the rest.
template <typename T, size_t LEN>
class IORingBuffer {
public:
  using buffer_t = emp::array<T, LEN>;

protected:
  buffer_t buffer;
  size_t next = 0;

public:
  // Construct buffer filled with 0s
  IORingBuffer() {
    Reset(0);
  }

  // Construct buffer filled with given fill value.
  IORingBuffer(T fill) {
    Reset(fill);
  }

  // Construct buffer with given contents
  IORingBuffer(const buffer_t& contents) : buffer(contents) { ; }

  // Push new value into buffer at "next" position, overwriting what was previously
  // there. Advances "next".
  void push(T x) {
    emp_assert(next < LEN);
    buffer[next] = x;
    next = ((x + 1) < LEN) ? x + 1 : 0;
  }

  // Index into ring buffer, default behavior is to wrap
  // TODO / QUESTION - Is this the default behavior that we want?
  // Can add a GetWrap function that does this and have this fail on out of range index.
  T operator[](size_t idx) const {
    // return buffer[idx % len];
    return buffer[(idx < LEN) ? idx : idx % LEN];
  }

  size_t size() const { return LEN; }

  // Reset contents of buffer to given fill value.
  void Reset(T fill_val) {
    std::fill(
      buffer.begin(),
      buffer.end(),
      fill_val
    );
  }

};

}