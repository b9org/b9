#if !defined(B9_CONTEXT_INL_HPP_)
#define B9_CONTEXT_INL_HPP_

#include <EnvironmentBase.hpp>
#include <b9/context.hpp>

namespace b9 {

inline MM_EnvironmentBase* Context::omrGcThread() const noexcept {
  return MM_EnvironmentBase::getEnvironment(omrVmThread());
}

}  // namespace b9

#endif  // B9_CONTEXT_INL_HPP_
