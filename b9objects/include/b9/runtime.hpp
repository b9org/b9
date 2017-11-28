#if !defined(B9_RUNTIME_HPP_)
#define B9_RUNTIME_HPP_

#include <omr.h>
#include <omrport.h>
#include <stdlib.h>
#include <string.h>
#include <thread_api.h>
#include <stdexcept>
// #include "GCExtensionsBase.hpp"
// #include "Heap.hpp"
#include "OMR_VMThread.hpp"
#include "StartupManagerImpl.hpp"
#include "mminitcore.h"
#include "omr.h"
#include "omrgcstartup.hpp"
#include "omrport.h"
#include "omrprofiler.h"
#include "omrrasinit.h"
#include "omrutil.h"
#include "omrvm.h"
#include "thread_api.h"
#include "omrvm.h"

namespace b9 {

class PlatformError : public std::exception {
 public:
  PlatformError(int error) : error_(error) {}

 private:
  int error_;
};

/// OMR Thread library wrapper
class ThreadInterface {
 public:
  static void init() {
    auto e = omrthread_init_library();
    if (e != 0) {
      throw PlatformError(e);
    }
  }

  static void tearDown() noexcept { omrthread_shutdown_library(); }

  ThreadInterface() { init(); }

  ~ThreadInterface() noexcept { tearDown(); }

 private:
};

/// omrthread wrapper.
class Thread {
 public:
  Thread(const ThreadInterface& ithread) {
    auto e = omrthread_attach_ex(&self_, J9THREAD_ATTR_DEFAULT);
    if (e != 0) {
      throw PlatformError(e);
    }
  }

  ~Thread() noexcept { omrthread_detach(self_); }

  omrthread_t data() { return self_; }

  const omrthread_t data() const { return self_; }

 private:
  omrthread_t self_;
};

/// OMR Thread and Port Library Wrapper
class PlatformInterface {
 public:
  PlatformInterface() {
    Thread self(threadInterface_);

    auto e = omrport_init_library(&library(), sizeof(OMRPortLibrary));
    if (e != 0) {
      throw PlatformError(e);
    }
  }

  ~PlatformInterface() noexcept {
    Thread self(threadInterface_);
    library().port_shutdown_library(&library());
  }

  OMRPortLibrary& library() noexcept {
    return portLibrary_;
  }

  const OMRPortLibrary& library() const noexcept {
    return portLibrary_;
  }

 private:
  ThreadInterface threadInterface_;
  OMRPortLibrary portLibrary_;
};

class ProcessRuntime {
 public:

  /// Initialize the process runtime.
  ProcessRuntime() {
    memset(&omrRuntime_, 0, sizeof(OMR_Runtime));
    omrRuntime_._configuration._maximum_vm_count = 0;
    omrRuntime_._vmCount = 0;
    omrRuntime_._portLibrary = &platform().library();

    auto e = omr_initialize_runtime(&omrRuntime_);
    if (e != 0) {
      throw PlatformError(e);
    }
  }

  ~ProcessRuntime() noexcept { omr_destroy_runtime(&omrRuntime_); }

  PlatformInterface& platform() { return platform_; }

  const PlatformInterface& platform() const { return platform_; }

  OMR_Runtime& omrRuntime() { return omrRuntime_; }

 private:
  PlatformInterface platform_;
  OMR_Runtime omrRuntime_;
};

class MemoryManager {
 public:
  explicit MemoryManager(ProcessRuntime& runtime) : runtime_(runtime) {
    memset(&omrVm_, 0, sizeof(OMR_VM));
    omrVm_._runtime = &runtime_.omrRuntime();
    omrVm_._language_vm = this;
  
    auto e = omr_attach_vm_to_runtime(&omrVm_);
    if (e != 0) {
      throw PlatformError(e);
    }
  }

  ~MemoryManager() { omr_detach_vm_from_runtime(&omrVm()); }

  OMR_VM& omrVm() { return omrVm_; }

  const OMR_VM& omrVm() const { return omrVm_; }

  ProcessRuntime& runtime() { return runtime_; }

 private:
  ProcessRuntime& runtime_;
  OMR_VM omrVm_;
};

}  // namespace b9

#endif  // B9_RUNTIME_HPP_
