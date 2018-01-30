#ifndef OMR_OM_MEMORYMANAGER_HPP_
#define OMR_OM_MEMORYMANAGER_HPP_

#include <OMR/Om/Cell.hpp>
#include <OMR/Om/EmptyObjectMap.hpp>
#include <OMR/Om/MarkingFn.hpp>
#include <OMR/Om/MetaMap.hpp>
#include <OMR/Om/RootRef.hpp>
#include <OMR/Om/Runtime.hpp>

#include <set>
#include <stdexcept>

namespace OMR {
namespace Om {

struct Cell;
struct MetaMap;
struct EmptyObjectMap;
struct ArrayBufferMap;

class Visitor;
class Context;
class RunContext;
class StartupContext;

class StartupError : public ::std::runtime_error {
  using runtime_error::runtime_error;
};

/// A collection of singleton GC cells. The collection is initialized at startup
class Globals {
 public:
  MetaMap* metaMap() const noexcept { return metaMap_; }

  EmptyObjectMap* emptyObjectMap() const noexcept { return emptyObjectMap_; }

  ArrayBufferMap* arrayBufferMap() const noexcept { return arrayBufferMap_; }

  template <typename VisitorT>
  void visit(Context& cx, VisitorT& visitor) {
    visitor.rootEdge(cx, this, (Cell*)metaMap_);
    visitor.rootEdge(cx, this, (Cell*)emptyObjectMap_);
    visitor.rootEdge(cx, this, (Cell*)arrayBufferMap_);
  }

 protected:
  friend class MemoryManager;

  /// Allocate the globals. Throws StartupError if any allocation fails.
  void init(StartupContext& cx);

 private:
  MetaMap* metaMap_ = nullptr;
  EmptyObjectMap* emptyObjectMap_ = nullptr;
  ArrayBufferMap* arrayBufferMap_ = nullptr;
};

using ContextSet = ::std::set<Context*>;

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

  template <typename VisitorT>
  void visit(Context& cx, VisitorT& visitor) {
    globals_.visit(cx, visitor);
    for (auto& fn : userRoots()) {
      fn(cx, visitor);
    }
  }

  const ContextSet& contexts() const { return contexts_; }

  ContextSet& contexts() { return contexts_; }

 protected:
  friend class Context;

 private:
  void initOmrVm();

  void initOmrGc();

  ProcessRuntime& runtime_;
  OMR_VM omrVm_;
  Globals globals_;
  MarkingFnVector userRoots_;
  ContextSet contexts_;
};

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_MEMORYMANAGER_HPP_
