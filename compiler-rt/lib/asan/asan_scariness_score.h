//===-- asan_scariness_score.h ----------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a part of AddressSanitizer, an address sanity checker.
//
// Compute the level of scariness of the error message.
// Don't expect any deep science here, just a set of heuristics that suggest
// that e.g. 1-byte-read-global-buffer-overflow is less scary than
// 8-byte-write-stack-use-after-return.
//
// Every error report has one or more features, such as memory access size,
// type (read or write), type of accessed memory (e.g. free-d heap, or a global
// redzone), etc. Every such feature has an int score and a string description.
// The overall score is the sum of all feature scores and the description
// is a concatenation of feature descriptions.
// Examples:
//  17 (4-byte-read-heap-buffer-overflow)
//  65 (multi-byte-write-stack-use-after-return)
//  10 (null-deref)
//
//===----------------------------------------------------------------------===//

#ifndef ASAN_SCARINESS_SCORE_H
#define ASAN_SCARINESS_SCORE_H

#include "asan_flags.h"
#include "sanitizer_common/sanitizer_common.h"
#include "sanitizer_common/sanitizer_libc.h"

namespace __asan {
class ScarinessScore {
 public:
  ScarinessScore() {}
  void Scare(int add_to_score, const char *reason) {
    if (descr[0])
      internal_strlcat(descr, "-", sizeof(descr));
    internal_strlcat(descr, reason, sizeof(descr));
    score += add_to_score;
  };
  int GetScore() const { return score; }
  const char *GetDescription() const { return descr; }
  void Print() {
    if (score && flags()->print_scariness)
      Printf("SCARINESS: %d (%s)\n", score, descr);
  }
  static void PrintSimple(int score, const char *descr) {
    ScarinessScore SS;
    SS.Scare(score, descr);
    SS.Print();
  }

 private:
  int score = 0;
  char descr[1024] = {0};
};

}  // namespace __asan

#endif  // ASAN_SCARINESS_SCORE_H
