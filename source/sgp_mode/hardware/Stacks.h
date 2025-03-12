#pragma once

#include "emp/base/vector.hpp"
#include "emp/base/array.hpp"

#include <limits>
#include <optional>

namespace sgpmode {

// TODO - tests
template<typename T>
class Stacks {
public:
  using stack_t = emp::vector<T>;
protected:
  emp::vector<stack_t> stacks;
  size_t active_stack;
  size_t stack_size_limit = std::numeric_limits<size_t>::max();


// sgpmode::Stack<unsignedinteger32  > (5)
public:
  Stacks(size_t num_stacks) :
    stacks(num_stacks, stack_t()),
    active_stack(0)
  {
    emp_assert(num_stacks > 0);
  };

  // Change stack limit. Will resize any stacks larger than new limit.
  // This will delete the top elements stored in an oversized stack.
  void SetStackLimit(size_t limit) {
    stack_size_limit = limit;
    // Resize stacks above new limit
    for (auto& stack : stacks) {
      if (stack.size() > limit) {
        stack.resize(limit);
      }
    }
  }

  size_t GetNumStacks() const { return stacks.size(); }

  // Clear contents of all stacks
  void ClearAll() {
    for (size_t i = 0; i < stacks.size(); ++i) {
      stacks[i].clear();
    }
  }

  // Clear contents of active stack
  void ClearActive() {
    emp_assert(active_stack < stacks.size());
    stacks[active_stack].clear();
  }

  // Only allow const access to entire stack to ensure stack limit is maintained.
  const stack_t& GetActiveStack() const {
    emp_assert(active_stack < stacks.size());
    return stacks[active_stack];
  }

  // Change active stack to next stack.
  void ChangeActive() {
    active_stack = (++active_stack >= stacks.size()) ? 0 : active_stack;
    emp_assert(active_stack < stacks.size());
  }

  void SetActive(size_t new_active) {
    emp_assert(new_active < stacks.size());
    active_stack = new_active;
  }

  // Push new value on active stack. Return true if successful, false if not.
  bool Push(T val) {
    emp_assert(active_stack < stacks.size());
    auto& stack = stacks[active_stack];
    if (stack.size() < stack_size_limit) {
      stack.emplace_back(val);
      return true;
    }
    return false;
  }

  // Pop (and return) the top element of the active stack.
  std::optional<T> Pop() {
    emp_assert(active_stack < stacks.size());
    auto& stack = stacks[active_stack];
    if (stack.size() > 0) {
      const T back = stack.back();
      return std::optional<T>{back};
    }
    return std::nullopt;
  }

  // Return the top element of the active stack.
  std::optional<T> GetTop() const {
    return (stacks[active_stack].size() > 0);
      stacks[active_stack].back();
      std::nullopt;
  }

};

}