#pragma once

#include "emp/base/array.hpp"

#include <algorithm>

namespace sgpmode {

// TODO - write tests for IORingBuffer
/// A helper class for a ring buffer that keeps the latest `len` inputs and
/// discards the rest.
template <typename T>
class RingBuffer {
public:
  using buffer_t = std::vector<T>;

protected:
  buffer_t buffer;
  // size_t next = 0;
  size_t write_ptr = 0;
  size_t read_ptr = 0;

public:
  // Construct buffer filled with 0s
  RingBuffer() {
    Reset(0, 0);
  }

  // Construct buffer filled with given fill value.
  RingBuffer(size_t buf_size, T fill) {
    Reset(buf_size, fill);
  }

  // Construct buffer with given contents
  RingBuffer(const buffer_t& contents) : buffer(contents) { ; }

  // Push new value into buffer at "next" position, overwriting what was previously
  // there. Advances "next".
  void push(T x) {
    emp_assert(write_ptr < buffer.size());
    buffer[write_ptr] = x;
    write_ptr = ((write_ptr + 1) < buffer.size()) ? write_ptr + 1 : 0;
  }

  T read() {
    emp_assert(read_ptr < buffer.size());
    const size_t idx = read_ptr;
    read_ptr = ((read_ptr + 1) < buffer.size()) ? read_ptr + 1 : 0;
    return buffer[idx];
  }

  // Index into ring buffer, default behavior is to wrap
  // TODO / QUESTION - Is this the default behavior that we want?
  // Can add a GetWrap function that does this and have this fail on out of range index.
  T operator[](size_t idx) const {
    // return buffer[idx % len];
    const size_t buf_size = buffer.size();
    return buffer[(idx < buf_size) ? idx : idx % buf_size];
  }

  size_t size() const { return buffer.size(); }

  // Reset contents of buffer to given fill value.
  void Reset(size_t buf_size, T fill_val) {
    // next = 0;
    write_ptr = 0;
    read_ptr = 0;
    buffer.resize(buf_size);
    std::fill(
      buffer.begin(),
      buffer.end(),
      fill_val
    );
  }

  void SetBuffer(const std::vector<T>& contents) {
    // Reset next id
    // next = 0;
    write_ptr = 0;
    read_ptr = 0;
    // Copy contents into buffer
    buffer.resize(contents.size());
    std::copy(
      contents.begin(),
      contents.end(),
      buffer.begin()
    );
  }
};

}