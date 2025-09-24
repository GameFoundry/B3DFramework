// Copyright 2019 The Marl Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "B3DUtilityPrerequisites.h"

#include <functional>
#include <memory>

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

namespace marl {

class OSFiber {
 public:
  inline ~OSFiber();

  // createFiberFromCurrentThread() returns a fiber created from the current
  // thread.
  static inline b3d::UPtr<OSFiber> createFiberFromCurrentThread();

  // createFiber() returns a new fiber with the given stack size that will
  // call func when switched to. func() must end by switching back to another
  // fiber, and must not return.
  static inline b3d::UPtr<OSFiber> createFiber(
      size_t stackSize,
      const std::function<void()>& func);

  // switchTo() immediately switches execution to the given fiber.
  // switchTo() must be called on the currently executing fiber.
  inline void switchTo(OSFiber*);

 private:
  static inline void WINAPI run(void* self);
  LPVOID fiber = nullptr;
  bool isFiberFromThread = false;
  std::function<void()> target;
};

OSFiber::~OSFiber() {
  if (fiber != nullptr) {
    if (isFiberFromThread) {
      ConvertFiberToThread();
    } else {
      DeleteFiber(fiber);
    }
  }
}

b3d::UPtr<OSFiber> OSFiber::createFiberFromCurrentThread() {
  auto out = b3d::B3DMakeUnique<OSFiber>();
  out->fiber = ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH);
  out->isFiberFromThread = true;
  B3D_ASSERT(out->fiber != nullptr && "ConvertThreadToFiberEx() failed.");
  return out;
}

b3d::UPtr<OSFiber> OSFiber::createFiber(
    size_t stackSize,
    const std::function<void()>& func) {
  auto out = b3d::B3DMakeUnique<OSFiber>();
  // stackSize is rounded up to the system's allocation granularity (typically
  // 64 KB).
  out->fiber = CreateFiberEx(stackSize - 1, stackSize, FIBER_FLAG_FLOAT_SWITCH,
                             &OSFiber::run, out.get());
  out->target = func;
  B3D_ASSERT(out->fiber != nullptr && "CreateFiberEx() failed");
  return out;
}

void OSFiber::switchTo(OSFiber* to) {
  SwitchToFiber(to->fiber);
}

void WINAPI OSFiber::run(void* self) {
  std::function<void()> func;
  std::swap(func, reinterpret_cast<OSFiber*>(self)->target);
  func();
}

}  // namespace marl
