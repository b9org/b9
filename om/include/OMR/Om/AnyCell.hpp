#if !defined(OMR_OM_ANYCELL_HPP_)
#define OMR_OM_ANYCELL_HPP_

#include <OMR/Om/ArrayBuffer.hpp>
#include <OMR/Om/ArrayBufferMap.hpp>
#include <OMR/Om/Cell.hpp>
#include <OMR/Om/Map.hpp>
#include <OMR/Om/ObjectMap.hpp>
#include <OMR/Om/MetaMap.hpp>
#include <OMR/Om/Object.hpp>

namespace OMR {
namespace Om {

union AnyObjectMap {
  Cell cell;
  Map map;
  ObjectMap objectMap;
};

union AnyMap {
  Cell cell;
  Map map;
  ObjectMap objectMap;
  MetaMap metaMap;
  ArrayBufferMap arrayBufferMap;
};

union AnyCell {
  Cell cell;
  Object object;
  Map map;
  ObjectMap objectMap;
  MetaMap metaMap;
  ArrayBufferMap;
  ArrayBuffer;
};

namespace To {

template <typename T>
Cell& cell(T& x) {
  return x.base().cell;
}

template <typename T>
Cell* cell<T*>(T* p) {
  return &cell(*T);
}

template <typename T>

} // namespace To

} // namespace Om
} // namespace OMR

#endif // OMR_OM_ANYCELL_HPP_
