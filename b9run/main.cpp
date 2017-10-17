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
    "Jit Options:\n"
    "  -jit:         Enable the jit\n"
    "  -directcall:  make direct jit to jit calls\n"
    "  -passparam:   Pass arguments in CPU registers\n"
    "  -lazyvmstate: Only update the VM state as needed\n"
    "Run Options:\n"
    "  -loop <n>:    Run the program <n> times\n"
    "  -inline <n>:  Enable inlining\n"
    "  -debug:       Enable debug code\n"
    "  -verbose:     Run with verbose printing\n"
    "  -help:        Print this help message";

/// The b9run program's global configuration.
struct RunConfig {
  b9::Config b9;
  const char* moduleName = "";
  const char* mainFunction = "b9main";
  std::size_t loopCount = 1;
  bool verbose = false;
  std::vector<b9::StackElement> usrArgs;
};

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
      cfg.b9.maxInlineDepth = atoi(argv[++i]);
    } else if (strcmp(arg, "-verbose") == 0) {
      cfg.verbose = true;
      cfg.b9.verbose = true;
    } else if (strcmp(arg, "-debug") == 0) {
      cfg.b9.debug = true;
    } else if (strcmp(arg, "-function") == 0) {
      cfg.mainFunction = argv[++i];
    } else if (strcmp(arg, "-jit") == 0) {
      cfg.b9.jit = true;
    } else if (strcmp(arg, "-directcall") == 0) {
      cfg.b9.directCall = true;
    } else if (strcmp(arg, "-passparam") == 0) {
      cfg.b9.passParam = true;
    } else if (strcmp(arg, "-lazyvmstate") == 0) {
      cfg.b9.lazyVmState = true;
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
  b9::VirtualMachine vm{cfg.b9};
  vm.initialize();

  b9::DlLoader loader{true};
  auto module = loader.loadModule(cfg.moduleName);

  if (module->functions.size() == 0) {
    throw b9::DlException{"Empty module"};
  }

  vm.load(module);

  size_t functionIndex = module->findFunction(cfg.mainFunction);
  if (cfg.loopCount == 1) {
    b9::StackElement returnVal = vm.run(functionIndex, cfg.usrArgs);
    std::cout << cfg.mainFunction << " returned: " << returnVal << std::endl;
  } else {
    b9::StackElement returnVal;
    std::cout << "Running " << cfg.mainFunction << " " << cfg.loopCount
              << " times:" << std::endl;
    for (std::size_t i = 1; i <= cfg.loopCount; i++) {
      returnVal = vm.run(functionIndex, cfg.usrArgs);
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
  } catch (const b9::BadFunctionCallException& e) {
    std::cerr << "Failed to call function " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
