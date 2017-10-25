#include <b9/interpreter.hpp>
#include <b9/jit.hpp>
#include <b9/loader.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

/// B9run's usage string. Printed when run with -help.
static const char* usage =
    "Usage: b9run [<option>...] [--] <module> [<arg>...]\n"
    "   Or: b9run -help\n"
    "Jit Options:\n"
    "  -jit:          Enable the jit\n"
    "  -directcall:   make direct jit to jit calls\n"
    "  -passparam:    Pass arguments in CPU registers\n"
    "  -lazyvmstate:  Only update the VM state as needed\n"
    "Run Options:\n"
    "  -function <f>: Run the function <f> (default: b9main)\n"
    "  -loop <n>:     Run the program <n> times (default: 1)\n"
    "  -inline <n>:   Set the jit's max inline depth (default: 1)\n"
    "  -debug:        Enable debug code\n"
    "  -verbose:      Run with verbose printing\n"
    "  -help:         Print this help message";

/// The b9run program's global configuration.
struct RunConfig {
  b9::Config b9;
  const char* moduleName = "";
  const char* mainFunction = "b9main";
  std::size_t loopCount = 1;
  bool verbose = false;
  std::vector<b9::StackElement> usrArgs;
};

std::ostream& operator<<(std::ostream& out, const RunConfig& cfg) {
  out << "Module:       " << cfg.moduleName << std::endl
      << "Function:     " << cfg.mainFunction << std::endl
      << "Looping:      " << cfg.loopCount << std::endl;

  out << "Arguments:    [ ";
  for (const auto& arg : cfg.usrArgs) {
    out << arg << " ";
  }
  out << "]" << std::endl;

  out << cfg.b9;
  return out;
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

  b9::DlLoader loader{true};
  auto module = loader.loadModule(cfg.moduleName);

  if (module->functions.size() == 0) {
    throw b9::DlException{"Empty module"};
  }

  vm.load(module);

  if (cfg.b9.jit) vm.generateAllCode();

  size_t functionIndex = module->findFunction(cfg.mainFunction);
  for (std::size_t i = 0; i < cfg.loopCount; i += 1) {
    auto result = vm.run(functionIndex, cfg.usrArgs);
    std::cout << std::endl << "=> " << result << std::endl;
  }
}

int main(int argc, char* argv[]) {
  RunConfig cfg;

  if (!parseArguments(cfg, argc, argv)) {
    std::cerr << usage << std::endl;
    exit(EXIT_FAILURE);
  }

  if (cfg.verbose) {
    std::cout << cfg << std::endl << std::endl;
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
  } catch (const b9::CompilationException& e) {
    std::cerr << "Failed to compile function: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
