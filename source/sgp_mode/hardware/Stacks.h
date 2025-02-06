#pragma once

#include "emp/base/vector.hpp"

#include "emp/base/array.hpp"

namespace sgpmode {

// TODO - tests
template<typename T>
class Stacks {
public:
  using stack_t = emp::vector<T>;
protected:
  emp::vector<stack_t> stacks;
  size_t active_stack;

public:
  Stacks(size_t num_stacks) :
    stacks(num_stacks, stack_t()),
    active_stack(0)
  {
    emp_assert(num_stacks > 0);
  };

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

  stack_t& GetActiveStack() {
    emp_assert(active_stack < stacks.size());
    return stacks[active_stack];
  }

  const stack_t& GetActiveStack() const {
    emp_assert(active_stack < stacks.size());
    return stacks[active_stack];
  }

  T GetTop() const {
    return stacks[active_stack].back();
  }

  // TODO - more functionality as needed

};

}