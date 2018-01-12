#ifndef OMR_OM_MEMORYMANAGER_HPP_
#define OMR_OM_MEMORYMANAGER_HPP_

#include <OMR/Om/Cell.hpp>
#include <OMR/Om/EmptyObjectMap.hpp>
#include <OMR/Om/MarkingFn.hpp>
#include <OMR/Om/MetaMap.hpp>
#include <OMR/Om/RootRef.hpp>
#include <OMR/Om/Runtime.hpp>

#include <unordered_set>

namespace OMR {
namespace Om {

class Visitor;

class Context;
class RunContext;
class StartupContext;

struct Globals {
  MetaMap* metaMap = nullptr;
  EmptyObjectMap* emptyObjectMap = nullptr;
};

using ContextSet = std::unordered_set<Context*>;

class MemoryManager {
 public:
  explicit MemoryManager(ProcessRuntime& runtime);

  MemoryManager(const MemoryManager&) = delete;

  ~MemoryManager();

  OMR_VM& omrVm() { return omrVm_; }

  const OMR_VM& omrVm() const { return omrVm_; }

  ProcessRuntime& runtime() { return runtime_; }

  const Globals& globals() const { return globals_; }

  MarkingFnVector userRoots() noexcept { return userRoots_; }

  const MarkingFnVector userRoots() const noexcept { return userRoots_; }

  void visitRoots(Context& cx, Visitor& visitor);

  const ContextSet& contexts() const { return contexts_; }

  ContextSet& contexts() { return contexts_; }

 protected:
  friend class Context;

 private:
  void initOmrVm();

  void initOmrGc();

  void initGlobals(StartupContext& cx);

  ProcessRuntime& runtime_;
  OMR_VM omrVm_;
  Globals globals_;
  MarkingFnVector userRoots_;
  ContextSet contexts_;
};

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_MEMORYMANAGER_HPP_
