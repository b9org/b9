#if !defined(OMR_OM_MEMORYMANAGER_INL_HPP_)
#define OMR_OM_MEMORYMANAGER_INL_HPP_

#include <OMR/Om/MemoryManager.hpp>

#include <OMR/Om/Allocator.inl.hpp>
#include <OMR/Om/Context.inl.hpp>
#include <OMR/Om/Map.hpp>
#include <OMR/Om/Runtime.hpp>
#include <OMR/Om/Traverse.hpp>

namespace OMR {
namespace Om {

inline MemoryManager::MemoryManager(ProcessRuntime& runtime)
    : runtime_(runtime) {
  Thread self(runtime.platform().thread());
  initOmrVm();
  initOmrGc();

  StartupContext cx(*this);
  OMR_GC_InitializeDispatcherThreads(cx.omrVmThread());

  initGlobals(cx);
}

inline MemoryManager::~MemoryManager() {
  Thread self(runtime().platform().thread());
  // TODO: Shut down the heap (requires a thread (boo!!))
  omr_detach_vm_from_runtime(&omrVm());
}

inline void MemoryManager::initOmrVm() {
  memset(&omrVm_, 0, sizeof(OMR_VM));
  omrVm_._runtime = &runtime_.omrRuntime();
  omrVm_._language_vm = this;
  omrVm_._vmThreadList = nullptr;
  omrVm_._gcOmrVMExtensions = nullptr;
  omrVm_._languageThreadCount = 0;

  auto e = omr_attach_vm_to_runtime(&omrVm_);

  if (e != 0) {
    throw PlatformError(e);
  }
}

inline void MemoryManager::initOmrGc() {
  MM_StartupManagerImpl startupManager(&omrVm_);
  auto e = OMR_GC_IntializeHeapAndCollector(&omrVm_, &startupManager);
  if (e != 0) {
    throw PlatformError(e);
  }
}

inline void MemoryManager::initGlobals(StartupContext& cx) {
  globals_.metaMap = allocateMetaMap(cx);
  globals_.emptyObjectMap = allocateEmptyObjectMap(cx);
}

inline void MemoryManager::visitRoots(Context& cx, Visitor& visitor) {
  visitor.rootEdge(cx, this, &globals_.metaMap->base.cell);
  visitor.rootEdge(cx, this, &globals_.emptyObjectMap->base.cell);

  for (auto& fn : userRoots()) {
    fn(cx, visitor);
  }
};

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_MEMORYMANAGER_INL_HPP_
