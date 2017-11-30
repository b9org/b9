#if !defined(B9_CONTEXT_INL_HPP_)
#define B9_CONTEXT_INL_HPP_

#include <EnvironmentBase.hpp>
#include <b9/context.hpp>

namespace b9 {

inline Context::Context(MemoryManager& manager) : manager_(manager) {
  auto e =
      OMR_Thread_Init(&manager.omrVm(), this, &omrVmThread_, "b9::Context");
  if (e != 0) throw std::runtime_error("Failed to attach OMR thread to OMR VM");
}

inline Context::~Context() noexcept {
  OMR_Thread_Free(omrVmThread_);
  // omrVmThread_ = nullptr_t;
}

inline MM_EnvironmentBase* Context::omrGcThread() const noexcept {
  return MM_EnvironmentBase::getEnvironment(omrVmThread());
}

inline Context& getContext(MM_EnvironmentBase& omrGcThread) {
	return getContext(omrGcThread.getOmrVMThread());
}

inline Context& getContext(MM_EnvironmentBase* omrGcThread) {
	return getContext(*omrGcThread);
}

}  // namespace b9

#endif  // B9_CONTEXT_INL_HPP_
