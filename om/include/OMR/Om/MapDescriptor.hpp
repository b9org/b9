

namespace b9 {

struct MapDescriptor {
	Map* parent;
	Id id;
};

constexpr bool operator!=(const MapDescriptor& lhs, const Map& rhs)
constexpr bool operator==(const MapDescriptor& lhs, const Map& rhs) const {
	return (lhs.parent == rhs.parent()) && (lhs.id == rhs.id());
}

constexpr bool operator==(const MapDescriptor& lhs, const MapDescriptor& rhs) const {
	return (lhs.parent == rhs.parent) && (lhs.id == rhs.id);
}

constexpr std::size_t hash(const MapDescriptor& d) {
	return mix(hash(d.parent), d.id.hash());
}

} // namespace b9

namespace std {
	template <>
	struct hash<b9::MapDescriptor> {
		constexpr std::size_t operator(const b9::MapDescriptor& d) const {
			return b9::hash(d);
		}
	};
} // namespace std
