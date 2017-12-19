# OMR Om: Meditations on Objects

(a dynamic object model for OMR)

## Getting Started

### Basic Usage

```c++
int main(int argc, char** argv) {
	OMR::Om::ProcessRuntime runtime;
	OMR::Om::MemoryManager manager(runtime);
	OMR::Om::Context(manager);
	Ref<Object> object = allocateEmptyObject(context);
}
```