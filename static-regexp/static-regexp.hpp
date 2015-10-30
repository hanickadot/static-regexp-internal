#ifndef STATIC_REGEXP_HPP
#define STATIC_REGEXP_HPP

#include <static-regexp/string.hpp>

namespace sre {

template <typename... T> struct StateHelper;
	
template <> struct StateHelper<> {
	void save() { }
	void load() const { }
};	

template <typename First, typename... Rest> struct StateHelper<First, Rest...> {
	First first;
	StateHelper<Rest...> rest;
	void save(const First & right, const Rest & ... lrest) {
		//printf("  obj: %p\n",&right);
		first = right;
		rest.save(lrest...);
	}
	void load(First & right, Rest & ... lrest) const {
		right = first;
		rest.load(lrest...);
	}
};

template <typename... T> struct State {
	StateHelper<T...> helper;
	void save(const T & ... rest) {
		//printf("%p: saving... (%zu) (%zu bytes)\n",this,sizeof...(T),sizeof(*this));
		helper.save(rest...);
	}
	void load(T & ... rest) const {
		//printf("%p: loading... (%zu) (%zu bytes)\n",this,sizeof...(T),sizeof(*this));
		helper.load(rest...);
	}
};
	

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
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringView<string_t> && view, Right & right, FarRight & ... fright) {
		return view.isBegin() && right(std::forward<sre::StringView<string_t>>(view), fright...);
	}
};

class End {
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringView<string_t> && view, Right & right, FarRight & ... fright) {
		return view.isEnd() && right(std::forward<sre::StringView<string_t>>(view), fright...);
	}
};

template <unsigned int... c> class Char {
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringView<string_t> && view, Right & right, FarRight & ... fright) {
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
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringView<string_t> && view, Right & right, FarRight & ... fright) {
		return !view.isEnd() && (RangeDefinition<def...>::isContains(*view)) && right(std::forward<sre::StringView<string_t>>(view).next(), fright...);
	}
};

template <typename... Parts> class Sequence;

template <> class Sequence<> {
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringView<string_t> && view, Right & right, FarRight & ... fright) {
		return right(std::forward<sre::StringView<string_t>>(view), fright...);
	}
};

template <typename First> class Sequence<First> {
protected:
	First first;
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringView<string_t> && view, Right & right, FarRight & ... fright) {
		return first(std::forward<sre::StringView<string_t>>(view), right, fright...);
	}
};

template <typename First, typename... Rest> class Sequence<First, Rest...> {
protected:
	First first;
	Sequence<Rest...> rest;
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringView<string_t> && view, Right & right, FarRight & ... fright) {
		return first(std::forward<sre::StringView<string_t>>(view), rest, right, fright...);
	}
};

template <typename... Options> class Select;

template <> class Select<> {
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringView<string_t> &&, Right &, FarRight & ...) const {
		return false;
	}
};

template <typename First> class Select<First> {
protected:
	First first;
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringView<string_t> && view, Right & right, FarRight & ... fright) {
		return first(std::forward<sre::StringView<string_t>>(view), right, fright...);
	}
};

template <typename First, typename... Rest> class Select<First, Rest...> {
protected:
	First first;
	Select<Rest...> rest;
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringView<string_t> && view, Right & right, FarRight & ... fright) {
		return first(std::forward<sre::StringView<string_t>>(view), right, fright...) || rest(std::forward<sre::StringView<string_t>>(view), right, fright...);
	}
};

template <typename... Inner> class Optional {
protected:
	Sequence<Inner...> inner;
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringView<string_t> && view, Right & right, FarRight & ... fright) {
		return inner(std::forward<sre::StringView<string_t>>(view), right, fright...) || right(std::forward<sre::StringView<string_t>>(view), fright...);
	}
};

template <unsigned int count, typename... Inner> class ExactRepeat {
protected:
	Sequence<Inner...> inner;
	template <typename string_t> struct Helper {
		mutable sre::StringView<string_t> storedView;
		bool operator()(sre::StringView<string_t> && view) const {
			storedView = std::move(view);
			return true;
		};
	};
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringView<string_t> && view, Right & right, FarRight & ... fright) {
		Helper<string_t> helper{std::forward<sre::StringView<string_t>>(view)};
		for (unsigned int i{0}; i != count; ++i) {
			if (!inner(std::move(helper.storedView), helper)) return false;
		}
		return right(std::move(helper.storedView), fright...);
	}
};

template <typename... Inner> class ExactRepeat<0,Inner...> {
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringView<string_t> && view, Right & right, FarRight & ... fright) {
		return right(std::forward<sre::StringView<string_t>>(view), fright...);
	}
};


template <unsigned int min, unsigned int max, typename... Inner> class Repeat {
protected:
	Sequence<Inner...> inner;
	template <typename string_t> struct Helper {
		mutable sre::StringView<string_t> storedView;
		bool operator()(sre::StringView<string_t> && view) const {
			storedView = std::move(view);
			return true;
		};
		void operator=(const Helper &) const {
			// this avoid copying
		}
	};
	bool validIteration(unsigned int i) const {
		return (min && max && (i >= min) && (i <= max)) || (min && !max && (i >= min)) || (!min && max && (i <= max)) || (!min && !max);
	}
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringView<string_t> && view, Right & right, FarRight & ... fright) {
		Helper<string_t> helper{std::forward<sre::StringView<string_t>>(view)};
		bool havePositive{false};
	
		//printf("cycle: %p (helper = %p)\n",this,&helper);
		State<Right, FarRight...> state{right, fright...};
		for (unsigned int i{0};; ++i) {
			
			if (max && max < i) break;
			else if (!min || (min && min <= i)) {
				state.save(right, fright...);
				
				
				if (right(helper.storedView.copy(),fright...)) {
					havePositive = true;
					state.load(right, fright...);
				}
			}
			
			if (!inner(std::move(helper.storedView), helper)) break;
		}
		if (havePositive) {
			state.load(right, fright...);
			return true;
		}
		return false;
	}
};

template <typename... Inner> using Plus = Repeat<1,0,Inner...>;
template <typename... Inner> using Star = Repeat<0,0,Inner...>;


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
