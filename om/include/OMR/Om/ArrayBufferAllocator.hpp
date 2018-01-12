#if !defined(OMR_OM_ARRAYBUFFERALLOCATOR_HPP_)
#define OMR_OM_ARRAYBUFFERALLOCATOR_HPP_

namespace OMR {
namespace Om {

class ArrayBufferAllocator {
	ArrayBuffer* allocate(Context& cx, std::size_t size) {
		return allocate(cx, cx.globals().arrayBufferMap, size);
	};

	ArrayBuffer* allocate(Context& cx, ArrayBufferMap* map, std::size_t size) {
		ArrayBufferInitializer init(map, size);
		return OMR_GC_Allocate(cx.omrVmThread(), init);
	}
};

}  // namespace Om
}  // namespace OMR

#endif // OMR_OM_ARRAYBUFFERALLOCATOR_HPP_
