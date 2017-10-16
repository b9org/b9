#include <b9.hpp>
#include <b9/loader.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

/// B9run's usage string. Printed when run with -help.
static const char* usage =
    "Usage: b9run [<option>...] [--] <module> [<main>]\n"
    "   Or: b9run -help\n"
    "Options:\n"
    "  -callstyle <style>: Set the calling style. One of:\n"
    "      interpreter:   Calls are made through the interpreter\n"
    "      direct:        Calls are made directly, but parameters are on the "
    "operand stack\n"
    "      passparameter: Direct calls, with parameters passed in CPU "
    "registers\n"
    "      operandstack:  Like passparam, but will keep the VM operand stack "
    "updated\n"
    "  -loop <n>:   Run the program <n> times\n"
    "  -inline <n>: Enable inlining\n"
    "  -debug:      Enable debug code\n"
    "  -verbose:    Run with verbose printing\n"
    "  -help:       Print this help message";

/// The b9run program's global configuration.
struct RunConfig {
  b9::VirtualMachineConfig vm;
  const char* moduleName = "";
  const char* mainFunction = "b9main";
  std::size_t loopCount = 1;
  bool verbose = false;
  std::vector<int> usrArgs;
};

/// Print the configuration summary.
std::ostream& operator<<(std::ostream& out, const RunConfig& cfg) {
  return out << "Loading:      " << cfg.moduleName << std::endl
             << "Executing:    " << cfg.mainFunction << std::endl
             << "Call Style:   " << cfg.vm.jitConfig.callStyle << std::endl
             << "Looping:      " << cfg.loopCount << " times" << std::endl
             << "Inline depth: " << cfg.vm.jitConfig.maxInlineDepth;
}

/// Parse CLI arguments and set up the config.
static bool parseArguments(RunConfig& cfg, const int argc, char* argv[]) {
  std::size_t i = 1;

  for (; i < argc; i++) {
    const char* arg = argv[i];

    if (strcmp(arg, "-help") == 0) {
      std::cout << usage << std::endl;
      exit(EXIT_SUCCESS);
    } else if (strcmp(arg, "-loop") == 0) {
      cfg.loopCount = atoi(argv[++i]);
    } else if (strcmp(arg, "-inline") == 0) {
      cfg.vm.jitConfig.maxInlineDepth = atoi(argv[++i]);
    } else if (strcmp(arg, "-verbose") == 0) {
      cfg.verbose = true;
      cfg.vm.verbose = true;
      cfg.vm.jitConfig.verbose = true;
    } else if (strcmp(arg, "-debug") == 0) {
      cfg.vm.debug = true;
      cfg.vm.jitConfig.debug = true;
    } else if (strcmp(arg, "-callstyle") == 0) {
      i += 1;
      auto callStyle = argv[i];
      if (strcmp("interpreter", callStyle) == 0) {
        cfg.vm.jitConfig.callStyle = b9::CallStyle::interpreter;
      } else if (strcmp("direct", callStyle) == 0) {
        cfg.vm.jitConfig.callStyle = b9::CallStyle::direct;
      } else if (strcmp("passparameter", callStyle) == 0) {
        cfg.vm.jitConfig.callStyle = b9::CallStyle::passParameter;
      } else if (strcmp("operandstack", callStyle) == 0) {
        cfg.vm.jitConfig.callStyle = b9::CallStyle::operandStack;
      }
    } else if (strcmp(arg, "-function") == 0) {
      cfg.mainFunction=argv[++i];
    } else if (strcmp(arg, "--") == 0) {
      i++;
      break;
    } else if (strcmp(arg, "-") == 0) {
      std::cerr << "Unrecognized option: " << arg << std::endl;
      return false;
    } else {
      break;
    }
  }

  // check for user defined module
  if (i < argc) {
    cfg.moduleName = argv[i++];
  } else {
    std::cerr << "No module name given to b9run" << std::endl;
    return false;
  }
  
  // check for user defined arguments
  for (; i < argc; i++) {
    cfg.usrArgs.push_back(std::atoi(argv[i]));
  }

  return true;
}

static void run(const RunConfig& cfg) {
  b9::VirtualMachine vm{cfg.vm};
  vm.initialize();

  b9::DlLoader loader{true};
  auto module = loader.loadModule(cfg.moduleName);

  if (module->functions.size() == 0) {
    throw b9::DlException{"Empty module"};
  }

  vm.load(module);

  size_t functionIndex = module->findFunction(cfg.mainFunction);
  if (cfg.loopCount == 1) {
    b9::StackElement returnVal = vm.run(functionIndex);
    std::cout << cfg.mainFunction << " returned: " << returnVal << std::endl;
  } else {
    b9::StackElement returnVal;
    std::cout << "Running " << cfg.mainFunction << " " << cfg.loopCount
              << " times:" << std::endl;
    for (std::size_t i = 1; i <= cfg.loopCount; i++) {
      returnVal = vm.run(functionIndex);
      std::cout << "Return value of iteration " << i << ": " << returnVal
                << std::endl;
    }
  }
}

int main(int argc, char* argv[]) {
  RunConfig cfg;

  if (!parseArguments(cfg, argc, argv)) {
    std::cerr << usage << std::endl;
    exit(EXIT_FAILURE);
  }

  try {
    run(cfg);
  } catch (const b9::DlException& e) {
    std::cerr << "Failed to load module: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  } catch (const b9::FunctionNotFoundException& e) {
    std::cerr << "Failed to find function: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
