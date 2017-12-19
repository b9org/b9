namespace b9 {

using WriteBarrier = ::MM_StandardWriteBarrier;	

class WriteBarrier {

};

class PreBarrier {

};

class PostBarrier {

};

template <typename T>
struct WriteBarrier {

};

template <typename T>
struct ReadBarrier {

};


template <typename T>
struct Barrier {
public:
	T& raw() { return value_; }

	T value_;
};

template <typename T>
struct BarrierRef {

};
