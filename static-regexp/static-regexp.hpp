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
	StateHelper(const First & right, const Rest & ... lrest): first{right}, rest{lrest...} { }
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
	const char * name{nullptr};
	StateHelper<T...> helper;
	State(const char * lname, T & ... rest): name{lname}, helper{rest...} {
		
	}
	void save(const T & ... rest) {
		//printf("%p, %s: saving... (%zu) (%zu bytes)\n",this,name,sizeof...(T),sizeof(*this));
		helper.save(rest...);
	}
	void load(T & ... rest) const {
		//printf("%p, %s: loading... (%zu) (%zu bytes)\n",this,name,sizeof...(T),sizeof(*this));
		helper.load(rest...);
	}
};

struct PositionPair {
	mutable unsigned int begin;
	mutable unsigned int end;
	unsigned int len() const {
		return end - begin;
	}
	template <typename string_t> StringRange<string_t> operator()(StringRef<string_t> && view) const {
		return StringRange<string_t>{view.getRef()+begin, view.getRef()+end};
	}
	template <typename string_t> StringRange<string_t> operator()(StringRef<string_t> & view) const {
		return StringRange<string_t>{view.getRef()+begin, view.getRef()+end};
	}
	template <typename string_t> StringRange<string_t> operator()(const string_t && str) const {
		return StringRange<string_t>{str.begin()+begin, str.begin()+end};
	}
	template <typename string_t> StringRange<string_t> operator()(const string_t & str) const {
		return StringRange<string_t>{str.begin()+begin, str.begin()+end};
	}
};

class CatchRange {
protected:
	mutable const PositionPair * ptrBegin{nullptr};
	mutable const PositionPair * ptrEnd{nullptr};
public:
	CatchRange() { }
	CatchRange(const PositionPair * lptrBegin, const PositionPair * lptrEnd): ptrBegin{lptrBegin}, ptrEnd{lptrEnd} { }
	const PositionPair * begin() const {
		return ptrBegin;
	}
	const PositionPair * end() const {
		return ptrEnd;
	}
	const PositionPair & operator[](size_t id) const {
		return *(ptrBegin+id);
	}
	size_t size() const {
		return ptrEnd - ptrBegin;
	}
};

template <size_t maxCount> class StaticMemory {
protected:
	size_t count{0};
	PositionPair list[maxCount];
public:
	void store(PositionPair && pair) {
		if (count < maxCount) {
			list[count++] = pair;
		}
	}
	const PositionPair * begin() const {
		return &list[0];
	}
	const PositionPair * end() const {
		return &list[count];
	}
	CatchRange getRange() const {
		return {begin(),end()};
	}
	void reset() {
		count = 0;
	}
	StaticMemory & operator=(const StaticMemory & right) {
		count = right.count;
		//printf("moving with memory: %p\n",this);
		for (unsigned int i{0}; i != maxCount; ++i) {
			list[i] = right.list[i];
		}
		return *this;
	}
};	

class DynamicMemory {
	
};

using OneMemory = StaticMemory<1>;

class PositiveClosure {
public:
	template <typename string_t> bool operator()(sre::StringRef<string_t> && view) {
		// there is no FarRight or Right, because Closure should be last thing on the right side :)
		return true;
	}
};

class Begin {
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringRef<string_t> && view, Right & right, FarRight & ... fright) {
		return view.isBegin() && right(std::forward<sre::StringRef<string_t>>(view), fright...);
	}
	template <unsigned int> size_t get(CatchRange &) const {
		return 0;
	}
	void resetMemory() { }
};

class End {
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringRef<string_t> && view, Right & right, FarRight & ... fright) {
		return view.isEnd() && right(std::forward<sre::StringRef<string_t>>(view), fright...);
	}
	template <unsigned int> size_t get(CatchRange &) const {
		return 0;
	}
	void resetMemory() { }
};

template <unsigned int... c> class Char {
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringRef<string_t> && view, Right & right, FarRight & ... fright) {
		return !view.isEnd() && ((*view == c) || ...) && right(std::forward<sre::StringRef<string_t>>(view).next(), fright...);
	}
	template <unsigned int> size_t get(CatchRange &) const {
		return 0;
	}
	void resetMemory() { }
};

using White = Char<'\n','\r','\t',' '>;

template <unsigned int... c> class NegChar {
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringRef<string_t> && view, Right & right, FarRight & ... fright) {
		return !view.isEnd() && ((*view != c) && ...) && right(std::forward<sre::StringRef<string_t>>(view).next(), fright...);
	}
	template <unsigned int> size_t get(CatchRange &) const {
		return false;
	}
	void resetMemory() { }
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
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringRef<string_t> && view, Right & right, FarRight & ... fright) {
		return !view.isEnd() && (RangeDefinition<def...>::isContains(*view)) && right(std::forward<sre::StringRef<string_t>>(view).next(), fright...);
	}
	template <unsigned int> size_t get(CatchRange &) const {
		return 0;
	}
	void resetMemory() { }
};

using Number = Range<'0','9'>;
using Alpha = Range<'a','z','A','Z'>;
using AlphaNumeric = Range<'a','z','A','Z','0','9'>;


template <typename... Parts> class Sequence;

template <> class Sequence<> {
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringRef<string_t> && view, Right & right, FarRight & ... fright) {
		return right(std::forward<sre::StringRef<string_t>>(view), fright...);
	}
	template <unsigned int> size_t get(CatchRange &) const {
		return 0;
	}
	void resetMemory() { }
};

template <typename First> class Sequence<First> {
protected:
	First first;
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringRef<string_t> && view, Right & right, FarRight & ... fright) {
		return first(std::forward<sre::StringRef<string_t>>(view), right, fright...);
	}
	template <unsigned int reqid> size_t get(CatchRange & cr) const {
		return first.template get<reqid>(cr);
	}
	void resetMemory() {
		first.resetMemory();
	}
};

template <typename First, typename... Rest> class Sequence<First, Rest...> {
protected:
	First first;
	Sequence<Rest...> rest;
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringRef<string_t> && view, Right & right, FarRight & ... fright) {
		return first(std::forward<sre::StringRef<string_t>>(view), rest, right, fright...);
	}
	template <unsigned int reqid> size_t get(CatchRange & cr) const {
		if (size_t sz = first.template get<reqid>(cr)) {
			return sz;
		} else {
			return rest.template get<reqid>(cr);
		}
	}
	void resetMemory() {
		first.resetMemory();
		rest.resetMemory();
	}
};

template <unsigned int... c> using String = Sequence<Char<c>...>;

template <typename... Options> class Select;

template <> class Select<> {
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringRef<string_t> &&, Right &, FarRight & ...) const {
		return false;
	}
	template <unsigned int reqid> size_t get(CatchRange & cr) const {
		return false;
	}
	void resetMemory() { }
};

template <typename First> class Select<First> {
protected:
	First first;
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringRef<string_t> && view, Right & right, FarRight & ... fright) {
		return first(std::forward<sre::StringRef<string_t>>(view), right, fright...);
	}
	template <unsigned int reqid> size_t get(CatchRange & cr) const {
		return first.template get<reqid>(cr);
	}
	void resetMemory() {
		first.resetMemory();
	}
};

template <typename First, typename... Rest> class Select<First, Rest...> {
protected:
	First first;
	Select<Rest...> rest;
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringRef<string_t> && view, Right & right, FarRight & ... fright) {
		State<Right, FarRight...> state{"select",right, fright...};
		if (first(std::forward<sre::StringRef<string_t>>(view), right, fright...)) {
			return true;
		} else {
			state.load(right, fright...);
			return rest(std::forward<sre::StringRef<string_t>>(view), right, fright...);
		}
	}
	template <unsigned int reqid> size_t get(CatchRange & cr) const {
		if (size_t sz = first.template get<reqid>(cr)) {
			return sz;
		} else {
			return rest.template get<reqid>(cr);
		}
	}
	void resetMemory() {
		first.resetMemory();
		rest.resetMemory();
	}
};

template <typename... Inner> class Optional {
protected:
	Sequence<Inner...> inner;
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringRef<string_t> && view, Right & right, FarRight & ... fright) {
		State<Right, FarRight...> state{"optional",right, fright...};
		if (inner(std::forward<sre::StringRef<string_t>>(view), right, fright...)) {
			return true;
		} else {
			state.load(right, fright...);
			return right(std::forward<sre::StringRef<string_t>>(view), fright...);
		}
	}
	template <unsigned int reqid> size_t get(CatchRange & cr) const {
		return inner.template get<reqid>(cr);
	}
	void resetMemory() {
		inner.resetMemory();
	}
};

template <unsigned int count, typename... Inner> class ExactRepeat {
protected:
	Sequence<Inner...> inner;
	template <typename string_t> struct Helper {
		mutable sre::StringRef<string_t> storedView;
		bool operator()(sre::StringRef<string_t> && view) const {
			storedView = std::move(view);
			return true;
		};
	};
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringRef<string_t> && view, Right & right, FarRight & ... fright) {
		Helper<string_t> helper{std::forward<sre::StringRef<string_t>>(view)};
		for (unsigned int i{0}; i != count; ++i) {
			if (!inner(std::move(helper.storedView), helper)) return false;
		}
		return right(std::move(helper.storedView), fright...);
	}
	template <unsigned int reqid> size_t get(CatchRange & cr) const {
		return inner.template get<reqid>(cr);
	}
	void resetMemory() {
		inner.resetMemory();
	}
};

template <typename... Inner> class ExactRepeat<0,Inner...> {
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringRef<string_t> && view, Right & right, FarRight & ... fright) {
		return right(std::forward<sre::StringRef<string_t>>(view), fright...);
	}
	template <unsigned int> size_t get(CatchRange &) const {
		return 0;
	}
	void resetMemory() { }
};


template <unsigned int min, unsigned int max, typename... Inner> class Repeat {
protected:
	Sequence<Inner...> inner;
	template <typename string_t> struct Helper {
		mutable sre::StringRef<string_t> storedView;
		bool operator()(sre::StringRef<string_t> && view) const {
			storedView = std::move(view);
			return true;
		};
		void operator=(const Helper &) const {
			// this is here to avoid copying
		}
	};
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringRef<string_t> && view, Right & right, FarRight & ... fright) {
		Helper<string_t> helper{std::forward<sre::StringRef<string_t>>(view)};
		bool havePositive{false};
	
		//printf("cycle: %p (helper = %p)\n",this,&helper);
		State<decltype(inner), Right, FarRight...> success{"success",inner, right, fright...};
		for (unsigned int i{0};; ++i) {
			
			if (max && max < i) break;
			else if (!min || (min && min <= i)) {
				State<decltype(inner), Right, FarRight...> state{"state",inner,right, fright...};
		
				if (right(helper.storedView.copy(),fright...)) {
					havePositive = true;
					success.save(inner, right, fright...);
					state.load(inner, right, fright...);
				}
			}
			
			if (!inner(helper.storedView.copy(), helper)) {
				break;
			}
		}
		if (havePositive) {
			success.load(inner, right, fright...);
			return true;
		}
		return false;
	}
	template <unsigned int reqid> size_t get(CatchRange & cr) const {
		return inner.template get<reqid>(cr);
	}
	void resetMemory() {
		inner.resetMemory();
	}
};

template <typename... Inner> using Plus = Repeat<1,0,Inner...>;
template <typename... Inner> using Star = Repeat<0,0,Inner...>;

template <unsigned int id, typename Storage, typename... Inner> class Catch {
protected:
	Sequence<Inner...> inner;
	struct Helper {
		unsigned int firstPos;
		Storage storage;
		Helper & operator=(const Helper &) = default;// {
		template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringRef<string_t> && view, Right & right, FarRight & ... fright) {
			unsigned int secondPos{view.getPosition()};
			if (right(std::forward<sre::StringRef<string_t>>(view), fright...)) {
				storage.store({firstPos,secondPos});
				return true;
			}
			else return false;
		};
	};
	Helper helper;
public:
	template <typename Right, typename... FarRight, typename string_t> bool operator()(sre::StringRef<string_t> && view, Right & right, FarRight & ... fright) {
		//printf("%u: get position: %zu: %c\n",id,view.getPosition(),*view);
		helper.firstPos = view.getPosition();
		return inner(std::forward<sre::StringRef<string_t>>(view), helper, right, fright...);
		
		//return inner(std::forward<sre::StringRef<string_t>>(view), right, fright...);
		//Helper<string_t> helper{std::forward<sre::StringRef<string_t>>(view)};
		//if (inner(std::move(helper.storedView), helper)) {
		//	storage.store({helper.getFirstPosition(), helper.getCurrentPosition()});
		//	return right(std::move(helper.storedView), fright...);
		//}
		//return false;
	}
	template <unsigned int reqid> size_t get(CatchRange & cr) const {
		if (id == reqid) {
			return (cr = helper.storage.getRange()).size();
		} else {
			return inner.template get<reqid>(cr);
		}
	}
	void resetMemory() {
		helper.storage.reset();
		inner.resetMemory();
	}
};

template <unsigned int id, typename... Inner> using OneCatch = Catch<id, OneMemory, Inner...>;
template <unsigned int id, size_t count, typename... Inner> using StaticCatch = Catch<id, StaticMemory<count>, Inner...>;
//template <typename... Inner> using DynamicCatch = Catch<DynamicMemory, Inner...>;


template <typename... Inner> class RegExp {
protected:
	Sequence<Inner...> inner;
public:
	template <typename string_t> bool match(sre::StringRef<string_t> && view) {
		PositiveClosure closure;
		inner.resetMemory();
		return inner(std::forward<sre::StringRef<string_t>>(view), closure);
	}
	template <typename string_t> bool match(sre::StringRef<string_t> & view) {
		PositiveClosure closure;
		inner.resetMemory();
		return inner(std::forward<sre::StringRef<string_t>>(view), closure);
	}
	template <typename string_t> bool match(const string_t && str) {
		return match(sre::make_sref(str));
	}
	template <typename string_t> bool match(const string_t & str) {
		return match(sre::make_sref(str));
	}
	template <unsigned int reqid> size_t get(CatchRange & cr) const {
		return inner.template get<reqid>(cr);
	}
};

}

#endif
