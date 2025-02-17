// Copyright 2021 gRPC authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef GRPC_CORE_LIB_PROMISE_MAP_H
#define GRPC_CORE_LIB_PROMISE_MAP_H

#include <grpc/impl/codegen/port_platform.h>

#include "absl/types/variant.h"
#include "src/core/lib/promise/detail/promise_like.h"
#include "src/core/lib/promise/poll.h"

namespace grpc_core {

namespace promise_detail {

// Implementation of mapping combinator - use this via the free function below!
// Promise is the type of promise to poll on, Fn is a function that takes the
// result of Promise and maps it to some new type.
template <typename Promise, typename Fn>
class Map {
 public:
  Map(Promise promise, Fn fn)
      : promise_(std::move(promise)), fn_(std::move(fn)) {}

  using Result = typename PromiseLike<Promise>::Result;

  Poll<Result> operator()() {
    Poll<Result> r = promise_();
    if (auto* p = absl::get_if<kPollReadyIdx>(&r)) {
      return fn_(std::move(*p));
    }
    return Pending();
  }

 private:
  PromiseLike<Promise> promise_;
  Fn fn_;
};

}  // namespace promise_detail

// Mapping combinator.
// Takes a promise, and a synchronous function to mutate its result, and
// returns a promise.
template <typename Promise, typename Fn>
promise_detail::Map<Promise, Fn> Map(Promise promise, Fn fn) {
  return promise_detail::Map<Promise, Fn>(std::move(promise), std::move(fn));
}

}  // namespace grpc_core

#endif  // GRPC_CORE_LIB_PROMISE_MAP_H
