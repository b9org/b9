---
layout: subchapter
title: Modules
---

In this section we're going to explain the module format, and simultaneous
write code to load a module.

A base9 module packages programs and libraries into a compact and portable
binary format. The frontend compiler is responsible for transforming our
javascript programs into modules, while the VM understands how to load and
execute the stored programs.

The module format starts with a small, fixed-sized header of metadata. The first
8 bytes contain the text "b9module", which helps us identify any file as a valid
module.

The module's main contents is divided into sections. Each section contains
a different type of information. For now, we'll only introduce two section
types: the string section and the function section.

Here we have the start of our Module loading program:

```c++
std::shared_ptr<Module> deserialize(std::istream &in) {
  auto module = std::make_shared<Module>();
  readHeader(in);
  while (in.peek() != std::istream::traits_type::eof()) {
    readSection(in, module);
  }
  return module;
}
```

## The Module header

Reading the header involves validating that the first 8 bytes match our magic
string, "b9module". Trying to load a file without the magic header present is an
error.

```c++
void validateHeader(std::istream &in) {
  if (in.peek() == std::istream::traits_type::eof()) {
    throw LoadError{"Empty input file"};
  }

  const char magic[] = {'b', '9', 'm', 'o', 'd', 'u', 'l', 'e'};
  const std::size_t nbytes = sizeof(magic);

  char buffer[nbytes];
  bool ok = readBytes(in, buffer, nbytes);

  if (!ok) {
    throw LoadError{"Failed to read header"};
  }
  if (strncmp(magic, buffer, nbytes) != 0) {
    throw LoadError{"Invalid header"};
  }
}
```

## Section header

Each section begins with a bit of identifying information, allowing the loader
to determine the type and size of the section.

```c++
void readSection(std::istream &in, std::shared_ptr<Module> &module) {
  uint32_t sectionCode;
  if (!readNumber(in, sectionCode)) {
    throw DeserializeException{"Error reading section code"};
  }

  switch (sectionCode) {
    case 1:
      return readFunctionSection(in, module->functions);
    case 2:
      return readStringSection(in, module->strings);
    default:
      throw DeserializeException{"Invalid Section Code"};
  }
}
```

## The String Section

Program constants have to be embedded in the bytecode stream. Some types of
constants cannot be expressed reasonably 

String literals, like "Hello, World!", must be embedded in the module, but
cannot be embedded directly into the bytecode stream.

The string section is identified via it's section code '2', which is
immediately followed by the number of strings in the section, a u32le value.
Strings are null-terminated, which means we have to search each string entry
for it's end. The code below accumulates the strings into a vector:

```c++
void readStringSection(std::istream &in, std::vector<std::string> &strings) {
  uint32_t stringCount;
  if (!readNumber(in, stringCount)) {
    throw DeserializeException{"Error reading string count"};
  }
  for (uint32_t i = 0; i < stringCount; i++) {
    std::string toRead;
    readString(in, toRead);
    strings.push_back(toRead);
  }
}
```

Eventually, we'll expand on the string section to introduce other types of
constant values.

## The Function Section

Like the string section, the function section starts with a u32le value indicating
the number of functions in the section.

```c++
void readFunctionSection(std::istream &in,
                         std::vector<FunctionDef> &functions) {
  uint32_t functionCount;
  if (!readNumber(in, functionCount)) {
    throw DeserializeException{"Error reading function count"};
  }
  for (uint32_t i = 0; i < functionCount; i++) {
    functions.emplace_back(FunctionDef{"", std::vector<Instruction>{}, 0, 0});
    readFunction(in, functions.back());
  }
}
```


```c++
void readFunctionSection(std::istream &in,
                         std::vector<FunctionDef> &functions);

void readFunction(std::istream &in, FunctionDef &functionDef);

void readFunctionData(std::istream &in, FunctionDef &functionDef);

bool readInstructions(std::istream &in,
                      std::vector<Instruction> &instructions);



void readFunction(std::istream &in, FunctionDef &functionDef) {
  readFunctionData(in, functionDef);
  if (!readInstructions(in, functionDef.instructions)) {
    throw DeserializeException{"Error reading instructions"};
  }
}

void readFunctionData(std::istream &in, FunctionDef &functionDef) {
  readString(in, functionDef.name);
  bool ok = readNumber(in, functionDef.nparams) &&
            readNumber(in, functionDef.nlocals);
  if (!ok) {
    throw DeserializeException{"Error reading function data"};
  }
}

bool readInstructions(std::istream &in,
                      std::vector<Instruction> &instructions) {
  do {
    RawInstruction instruction;
    if (!readNumber(in, instruction)) {
      return false;
    }
    instructions.emplace_back(instruction);
  } while (instructions.back() != END_SECTION);
  return true;
}
```

## Putting it all together

```c++
std::shared_ptr<Module> deserialize(std::istream &in) {
  auto module = std::make_shared<Module>();
  readHeader(in);
  while (in.peek() != std::istream::traits_type::eof()) {
    readSection(in, module);
  }
  return module;
}
```
