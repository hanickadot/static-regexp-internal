#ifndef STATIC_REGEXP_HPP
#define STATIC_REGEXP_HPP

#include <static-regexp/string.hpp>

namespace sre {

class PositiveClosure {
public:
	template <typename string_t> bool operator()(sre::StringView<string_t> && view) {
		// there is no FarRight or Right, because Closure should be last thing on the right side :)
		return true;
	}
};

template <typename Right, typename... FarRight, typename string_t> bool callFarRightContext(sre::StringView<string_t> && view, FarRight & ... right) {
	
}

class Begin {
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringView<string_t> && view, Right & right, FarRight & ... fright) const {
		return view.isBegin() && right(std::forward<sre::StringView<string_t>>(view), fright...);
	}
};

class End {
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringView<string_t> && view, Right & right, FarRight & ... fright) const {
		return view.isEnd() && right(std::forward<sre::StringView<string_t>>(view), fright...);
	}
};

template <unsigned int... c> class Char {
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringView<string_t> && view, Right & right, FarRight & ... fright) const {
		return !view.isEnd() && ((*view == c) || ...) && right(std::forward<sre::StringView<string_t>>(view).next(), fright...);
	}
};

template <unsigned int...> struct RangeDefinition;

template <unsigned int min, unsigned int max, unsigned int... rest> struct RangeDefinition<min, max, rest...> {
	template <typename value_type> static bool isContains(value_type c) {
		return ((min <= c) && (c <= max)) || RangeDefinition<rest...>::isContains(c);
	}
};

template <> struct RangeDefinition<> {
	template <typename value_type> static bool isContains(value_type) {
		return false;
	}
};

template <unsigned int... def> class Range {
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringView<string_t> && view, Right & right, FarRight & ... fright) const {
		return !view.isEnd() && (RangeDefinition<def...>::isContains(*view)) && right(std::forward<sre::StringView<string_t>>(view).next(), fright...);
	}
};

template <typename... Parts> class Sequence;

template <> class Sequence<> {
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringView<string_t> && view, Right & right, FarRight & ... fright) const {
		return right(std::forward<sre::StringView<string_t>>(view), fright...);
	}
};

template <typename First> class Sequence<First> {
protected:
	First first;
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringView<string_t> && view, Right & right, FarRight & ... fright) const {
		return first(std::forward<sre::StringView<string_t>>(view), right, fright...);
	}
};

template <typename First, typename... Rest> class Sequence<First, Rest...> {
protected:
	First first;
	Sequence<Rest...> rest;
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringView<string_t> && view, Right & right, FarRight & ... fright) const {
		return first(std::forward<sre::StringView<string_t>>(view), rest, right, fright...);
	}
};

template <typename... Inner> class RegExp {
public:
	template <typename string_t> static bool match(sre::StringView<string_t> && view) {
		PositiveClosure closure;
		return Sequence<Inner...>{}(std::forward<sre::StringView<string_t>>(view), closure);
	}
	template <typename string_t> static bool match(sre::StringView<string_t> & view) {
		PositiveClosure closure;
		return Sequence<Inner...>{}(std::forward<sre::StringView<string_t>>(view), closure);
	}
	template <typename string_t> static bool match(const string_t && str) {
		return match(sre::make_view(str));
	}
	template <typename string_t> static bool match(const string_t & str) {
		return match(sre::make_view(str));
	}
};

}

#endif
