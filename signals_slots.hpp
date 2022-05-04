#ifndef __SIGNALS_SLOTS_HPP_
#define __SIGNALS_SLOTS_HPP_
#include <functional>
#include <map>

#define signal public
#define slot public
#define emit
using uint64 = unsigned long long;
template<typename ...T>
struct Signal
{
	using Slot = std::function<void(T...)>;
	~Signal() {};
	uint64 bind(const Slot& functor)
	{
		uint64 next_id = m_next_id + 1;
		if (next_id < 0)
			return -1;//error
		if (!m_slots.insert(std::make_pair(next_id, functor)).second)//error
			return -1;
		m_next_id = next_id;
		return m_next_id;
	}
	void unbind(uint64 slot_id)
	{
		m_slots.erase(slot_id);
	}
	void operator()(T... arg)
	{
		for (auto& functor : m_slots)
		{
			functor.second(std::forward<T>(arg)...);
		}
	}
private:
	uint64 m_next_id = 0;//自增ID
	std::map<uint64, Slot> m_slots;//槽
};

template<typename ...T>
void disconnect(Signal<T...>& sig, uint64 slot_id)
{
	sig.unbind(slot_id);
}
//绑定lambda,非成员函数
template<typename SIG, typename Functor>
uint64 connect(SIG& sig, Functor functor)
{
	return sig.bind(functor);
}
#ifdef _MSC_VER
#define PLACE_HOLDER std::_Ph
#else
#define PLACE_HOLDER std::_Placeholder
#endif
#if __cplusplus >= 201402L

template<size_t... _Idx>
using Indexes = std::index_sequence<_Idx...>;
template<size_t _Num>
using GenIdx = std::make_index_sequence<_Num>;

#else

template<size_t ...I> struct Indexes {};
template<size_t  N, size_t  ...I>
struct GenIdx : public GenIdx<N - 1, N - 1, I...> {};
template<size_t  ...I>
struct GenIdx<0, I...> : public Indexes<I...> {};
#endif
namespace connect_helper {
template<typename RCV, typename CLS, typename ...T, size_t  ...I>
uint64 connect(Signal<T...>& sig, RCV* ptr, void(CLS::* method)(T...), const Indexes<0, I...>& is)
{
	return sig.bind(std::bind(method, ptr, PLACE_HOLDER<I>{}...));
}
template<typename RCV, typename CLS, typename ...T>
uint64 connect(Signal<T...>& sig, RCV* ptr, void(CLS::* method)(T...), const Indexes<0>& is)
{
	return sig.bind(std::bind(method, ptr));
}
}
//绑定成员函数
template<typename RCV, typename CLS, typename ...T>
uint64 connect(Signal<T...>& sig, RCV* ptr, void (CLS::* method)(T...))
{
	return connect_helper::connect(sig, ptr, method, GenIdx<sizeof...(T) + 1>());
}
#endif