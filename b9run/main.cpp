#include <b9.hpp>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

/// B9run's usage string. Printed when run with -help.
static const char* usage =
  "Usage: b9run [<option>...] [--] [<module> [<main>]]\n"
  "   Or: b9run -help\n"
  "Options:\n"
  "  -callstyle <style>: Set the calling style. One of:\n"
  "      interpreter:   Calls are made through the interpreter\n"
  "      direct:        Calls are made directly, but parameters are on the operand stack\n"
  "      passparameter: Direct calls, with parameters passed in CPU registers\n"
  "      operandstack:  Like passparam, but will keep the VM operand stack updated\n"
  "  -loop <n>:   Run the program <n> times\n"
  "  -inline <n>: Enable inlining\n"
  "  -debug:      Enable debug code\n"
  "  -verbose:    Run with verbose printing\n"
  "  -help:       Print this help message";

/// The b9run program's global configuration.
struct RunConfig {
  b9::VirtualMachineConfig vm;
  const char* module = "program.so";
  const char* mainFunction = "b9main";
  std::size_t loopCount = 1;
  bool verbose = false;
};

/// Print the configuration summary.
std::ostream& operator <<(std::ostream& out, const RunConfig& cfg) {
  return out
    << "Loading:      " << cfg.module                    << std::endl
    << "Executing:    " << cfg.mainFunction              << std::endl
    << "Looping:      " << cfg.loopCount << " times"     << std::endl
    << "Inline depth: " << cfg.vm.jit.maxInlineDepth     << std::endl
    << "Call Style:   " << cfg.vm.jit.callStyle;
}

/// Parse CLI arguments and set up the config.
static bool parseArguments(RunConfig& cfg, const int argc, char* argv[]) {
  /* Command Line Arguments */
  for (int i = 1; i < argc; i++) {
    const char *arg = argv[i];

    if (strcmp(arg, "-help") == 0) {
      std::cout << usage << std::endl;
      exit(EXIT_SUCCESS);
    }
    else if (strcmp(arg, "-loop") == 0) {
      cfg.loopCount = atoi(argv[++i]);
    }
    else if (strcmp(arg, "-inline") == 0) {
      cfg.vm.jit.maxInlineDepth = atoi(argv[++i]);
    }
    else if (strcmp(arg, "-verbose") == 0) {
      cfg.verbose = true;
      cfg.vm.verbose = true;
      cfg.vm.jit.verbse = true;
    }
    else if (strcmp(arg, "-debug") == 0) {
      cfg.vm.debug = true;
      cfg.vm.jit.debug = true;
    }
    else if (strcmp(arg, "-callstyle") == 0) {
      i += 1;
      auto callStyle = argv[i];
      if (strcmp("interpreter", callStyle) == 0) {
        cfg.vm.jit.callStyle = b9::CallStyle::interpreter;
      }
      else if (strcmp("direct", callStyle) == 0) {
        cfg.vm.jit.callStyle = b9::CallStyle::direct;
      }
      else if (strcmp("passparameter", callStyle) == 0) {
        cfg.vm.jit.callStyle = b9::CallStyle::passParameter;
      }
      else if (strcmp("operandstack", callStyle) == 0) {
        cfg.vm.jit.callStyle = b9::CallStyle::operandStack;
      }
    }
    else if (strcmp(arg, "-program") == 0) {
      cfg.mainFunction = argv[++i];
    }
    else if (strcmp(arg, "--") == 0) {
      return true;
    }
    else {
      std::cerr << "Unrecognized option: " << arg << std::endl;
      return false;
    }
  }
  return true;
}

static bool run(const RunConfig& cfg) {
  b9::VirtualMachine virtualMachine{cfg.vm};
  virtualMachine.initialize();

#if 0
 b9::VirtualMachine virtualMachine;
  char sharelib[128];

  virtualMachine.initialize();

  if (!virtualMachine.loadLibrary()) {
    return EXIT_FAILURE;
  }

  b9::Instruction *function = virtualMachine.getFunctionAddress(mainFunction);
  if (function == nullptr) {
    return EXIT_FAILURE;
  }

  b9::StackElement resultInterp = 0;
  b9::StackElement resultJit = 0;
  long timeInterp = 0;
  long timeJIT = 0;

  
  printf("Running Interpreted");
  // printf(
  //     "Options: DirectCall (%d), DirectParameterPassing (%d), "
  //     "UseVMOperandStack (%d)\n",
  //     virtualMachine.directCall_, virtualMachine.passParameters_, virtualMachine.operandStack_);

  resultInterp =
      timeFunction(&virtualMachine, function, 1,  &timeInterp);

  printf("Running JIT looping %d times\n", 1);
  //generateAllCode(&context);

  //resultJit = timeFunction(&context, function, context.loopCount, &timeJIT);

  printf("Result for Interp is %lld, resultJit is %lld\n", resultInterp, resultJit);

  printf("Time for Interp %ld ms, JIT %ld ms\n", timeInterp, timeJIT);
  printf("JIT speedup = %f\n", timeInterp * 1.0 / timeJIT);

  if (resultInterp == resultJit) {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
#endif // 0
}

int main(int argc, char* argv[]) {
  RunConfig cfg;

  if (parseArguments(cfg, argc, argv)) {
    exit(EXIT_FAILURE);
  }

  if (cfg.vm.verbose) {
    std::cout << cfg << std::endl;
  }

  if (!run(cfg)) {
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
