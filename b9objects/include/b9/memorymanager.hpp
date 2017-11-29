#if !defined(B9_MEMORYMANAGER_HPP_)
#define B9_MEMORYMANAGER_HPP_

#include <b9/runtime.hpp>

namespace b9 {

class Cell;
class Map;
class MapMap;
class EmptyObjectMap;
class Object;
class Visitor;

class Context;
class RunContext;
class StartupContext;

struct Globals {
  MapMap* mapMap;
  EmptyObjectMap* emptyObjectMap;
};

class MemoryManager {
 public:
  explicit MemoryManager(ProcessRuntime& runtime);

  ~MemoryManager();

  OMR_VM& omrVm() { return omrVm_; }

  const OMR_VM& omrVm() const { return omrVm_; }

  ProcessRuntime& runtime() { return runtime_; }

  const Globals& globals() const { return globals_; }

  void visitRoots(Context& cx, Visitor& visitor);

 private:
  void initOmrVm();

  void initOmrGc();

  void initGlobals(StartupContext& cx);

  ProcessRuntime& runtime_;
  OMR_VM omrVm_;
  Globals globals_;
};

} // namespace b9

#endif // B9_MEMORYMANAGER_HPP_
