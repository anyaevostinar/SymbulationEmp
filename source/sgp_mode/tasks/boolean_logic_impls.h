#pragma once

#include <cstdint>

namespace sgpmode::logic {

// TODO - write tests

//////////////////////////////////////////////
// One and two-input
uint32_t ECHO(uint32_t a) {
  return a;
}

uint32_t NOT(uint32_t a) {
  return ~a;
}

uint32_t NAND(uint32_t a, uint32_t b) {
  return ~(a&b);
}

uint32_t OR_NOT(uint32_t a, uint32_t b) {
  return (a|(~b));
}

uint32_t AND(uint32_t a, uint32_t b) {
  return (a&b);
}

uint32_t OR(uint32_t a, uint32_t b) {
  return (a|b);
}

uint32_t AND_NOT(uint32_t a, uint32_t b) {
  return (a&(~b));
}

uint32_t NOR(uint32_t a, uint32_t b) {
  return ~(a|b);
}

uint32_t XOR(uint32_t a, uint32_t b) {
  return (a^b);
}

uint32_t EQU(uint32_t a, uint32_t b) {
  return ~(a^b);
}

//////////////////////////////////////////////
// 3-input

} // end logic namespace