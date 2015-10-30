#ifndef STATIC_REGEXP__STRING_HPP
#define STATIC_REGEXP__STRING_HPP

namespace sre {
template <typename T> class StringView {
protected:
	const T & ref;
	typename T::const_iterator current;
public:
	using const_iterator = typename T::const_iterator;
	using value_type = typename T::value_type;
	StringView(const T & source): ref{source}, current{ref.cbegin()} { }
	StringView(const StringView & orig): ref{orig.ref}, current{orig.current} { }
	StringView(const StringView & orig, size_t move): ref{orig.ref}, current{orig.current+move} { }
	StringView(StringView && orig): ref{orig.ref}, current{orig.current} { }
	~StringView() = default;
	size_t getPosition() const {
		return current - ref.cbegin();
	}
	bool isBegin() const {
		return current == ref.cbegin();
	}
	bool isEnd() const {
		return current == ref.cend();
	}
	value_type get() const {
		return *current;
	}
	StringView copy() const {
		return StringView(*this);
	}
	StringView next() const {
		return StringView(*this,1);
	}
	const value_type & operator*() const {
		return *current;
	}
	StringView & operator=(StringView && right) {
		current = right.current;
		return *this;
	}
	StringView & operator=(const StringView & right) {
		current = right.current;
		return *this;
	}
	const_iterator begin() const {
		return current;
	}
	const_iterator end() const {
		return ref.cend();
	}
};

template <typename T> StringView<T> make_view(T & ref) {
	return StringView<T>{ref};
}

template <typename T> StringView<T> make_view(T && ref) {
	return StringView<T>{ref};
}

}

#endif