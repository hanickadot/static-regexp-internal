#ifndef STATIC_REGEXP__STRING_HPP
#define STATIC_REGEXP__STRING_HPP

namespace sre {
	
template <typename T> class StringRange {
public:
	using const_iterator = typename T::const_iterator;
protected:
	const_iterator beginValue;
	const_iterator endValue;
public:
	StringRange(const_iterator begin, const_iterator end): beginValue{begin}, endValue{end} { }
	const_iterator begin() const {
		return beginValue;
	}
	const_iterator end() const {
		return endValue;
	}
};

template <typename T> class StringRef {
public:
	using const_iterator = typename T::const_iterator;
	using value_type = typename T::value_type;
protected:
	const T & ref;
	const_iterator current;
public:
	StringRef(const T & source): ref{source}, current{ref.cbegin()} { }
	StringRef(const StringRef & orig): ref{orig.ref}, current{orig.current} { }
	StringRef(const StringRef & orig, size_t move): ref{orig.ref}, current{orig.current+move} { }
	StringRef(StringRef && orig): ref{orig.ref}, current{orig.current} { }
	~StringRef() = default;
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
	StringRef copy() const {
		return StringRef(*this);
	}
	StringRef next() const {
		return StringRef(*this,1);
	}
	const T & getRef() const {
		return ref;
	}
	const value_type & operator*() const {
		return *current;
	}
	StringRef & operator=(StringRef && right) {
		current = right.current;
		return *this;
	}
	StringRef & operator=(const StringRef & right) {
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

template <typename T> StringRef<T> make_sref(T & ref) {
	return StringRef<T>{ref};
}

template <typename T> StringRef<T> make_sref(T && ref) {
	return StringRef<T>{ref};
}

}

template <typename T> std::ostream & operator<<(std::ostream & stream, const sre::StringRange<T> & range) {
	for (auto & c: range) {
		stream << c;
	}
	return stream;
}

#endif