#pragma once
#include "pch.hpp"

template <typename KeyType, typename ObjType, bool maxHeap>
class Heap
{
	public:
	typedef std::pair<KeyType, ObjType> pair_type;

	private:
	size_t m_size = 0;
	size_t m_capacity = 0;
	pair_type* m_data = nullptr;

	void Expand();

	public:
	Heap() = default;
	~Heap() { Clear(true); }

	Heap(const Heap& ref);
	auto operator=(const Heap& ref) -> Heap&;

	Heap(Heap&& ref) noexcept;
	auto operator=(Heap&& ref) noexcept -> Heap&;

	void Pop();
	void Clear(bool reset = false);
	void Push(KeyType key, const ObjType& object);

	auto Top() const -> ObjType&;
	auto GetSize() const -> size_t;
	auto GetData() const -> pair_type*;
	auto GetCapacity() const -> size_t;
};

template <typename KeyType, typename ObjType>
using MinHeap = Heap<KeyType, ObjType, false>;

template <typename KeyType, typename ObjType>
using MaxHeap = Heap<KeyType, ObjType, true>;

template <typename KeyType, typename ObjType, bool maxHeap>
void Heap<KeyType, ObjType, maxHeap>::Expand()
{
	if (m_data != nullptr)
	{
		m_capacity <<= 1;

		pair_type* prevArr = m_data + m_size;
		pair_type* newArr = static_cast<pair_type*>(operator new(sizeof(pair_type) * m_capacity)) + m_size;

		while (prevArr != m_data)
		{
			--prevArr;
			--newArr;

			*newArr = std::move(*prevArr);
		}

		operator delete[](m_data);
		m_data = newArr;
	}
	else
	{
		m_capacity = 1;
		m_data = static_cast<pair_type*>(operator new(sizeof(pair_type)));
	}
}

template <typename KeyType, typename ObjType, bool maxHeap>
Heap<KeyType, ObjType, maxHeap>::Heap(const Heap& ref) : m_size(ref.m_size), m_capacity(ref.m_capacity)
{
	pair_type* refArr = ref.m_data + ref.m_size;
	m_data = static_cast<pair_type*>(operator new(sizeof(pair_type) * m_capacity)) + m_size;

	while (refArr != ref.m_data)
	{
		--m_data;
		--refArr;

		*m_data = *refArr;
	}
}

template <typename KeyType, typename ObjType, bool maxHeap>
auto Heap<KeyType, ObjType, maxHeap>::operator=(const Heap& ref) -> Heap&
{
	if (&ref == this) return *this;
	Clear(true);

	m_size = ref.m_size;
	m_capacity = ref.m_capacity;

	pair_type* refArr = ref.m_data + ref.m_size;
	m_data = static_cast<pair_type*>(operator new(sizeof(pair_type) * m_capacity)) + m_size;

	while (refArr != ref.m_data)
	{
		--m_data;
		--refArr;

		*m_data = *refArr;
	}

	return *this;
}

template <typename KeyType, typename ObjType, bool maxHeap>
Heap<KeyType, ObjType, maxHeap>::Heap(Heap&& ref) noexcept : m_size(ref.m_size), m_capacity(ref.m_capacity), m_data(ref.m_data)
{
	ref.m_size = 0;
	ref.m_capacity = 0;
	ref.m_data = nullptr;
}

template <typename KeyType, typename ObjType, bool maxHeap>
auto Heap<KeyType, ObjType, maxHeap>::operator=(Heap&& ref) noexcept -> Heap&
{
	if (&ref == this) return *this;
	Clear(true);

	m_size = ref.m_size;
	m_capacity = ref.m_capacity;
	m_data = ref.m_data;

	ref.m_size = 0;
	ref.m_capacity = 0;
	ref.m_data = nullptr;

	return *this;
}

template <typename KeyType, typename ObjType, bool maxHeap>
void Heap<KeyType, ObjType, maxHeap>::Pop()
{
	if (m_size == 0) return;

	m_data -> ~pair_type();
	if (--m_size == 0) return;

	*m_data = std::move(m_data[m_size]);
	size_t crrIndex = 0;

	while (true)
	{
		size_t chdIndex = (crrIndex << 1) + 1;
		if (const size_t rIndex = chdIndex + 1; rIndex < m_size)
		{
			if constexpr (maxHeap)
			{
				if (m_data[rIndex].first > m_data[chdIndex].first) chdIndex = rIndex;
			}
			else
			{
				if (m_data[rIndex].first < m_data[chdIndex].first) chdIndex = rIndex;
			}
		}
		else if (chdIndex >= m_size) break;

		pair_type& crrPair = m_data[crrIndex];
		pair_type& chdPair = m_data[chdIndex];

		if constexpr (maxHeap)
		{
			if (crrPair.first >= chdPair.first) break;
		}
		else
		{
			if (crrPair.first <= chdPair.first) break;
		}

		crrIndex = chdIndex;
		std::swap(crrPair, chdPair);
	}
}

template <typename KeyType, typename ObjType, bool maxHeap>
void Heap<KeyType, ObjType, maxHeap>::Clear(bool reset)
{
	if (m_data == nullptr) return;

	pair_type* iter = m_data + m_size;
	while (iter != m_data)
	{
		--iter;
		iter -> ~pair_type();
	}

	m_size = 0;
	if (reset)
	{
		operator delete[](m_data);
		m_data = nullptr;
		m_capacity = 0;
	}
}

template <typename KeyType, typename ObjType, bool maxHeap>
void Heap<KeyType, ObjType, maxHeap>::Push(KeyType key, const ObjType& object)
{
	if (m_size >= m_capacity) Expand();

	new(m_data + m_size) pair_type(key, object);
	size_t crrIndex = m_size;

	m_size++;
	while (crrIndex > 0)
	{
		const size_t parIndex = (crrIndex - 1) >> 1;

		pair_type& parPair = m_data[parIndex];
		pair_type& crrPair = m_data[crrIndex];

		if constexpr (maxHeap)
		{
			if (parPair.first >= crrPair.first) break;
		}
		else
		{
			if (parPair.first <= crrPair.first) break;
		}

		crrIndex = parIndex;
		std::swap(crrPair, parPair);
	}
}

template <typename KeyType, typename ObjType, bool maxHeap>
inline auto Heap<KeyType, ObjType, maxHeap>::Top() const -> ObjType&
{
	return m_data -> second;
}

template <typename KeyType, typename ObjType, bool maxHeap>
inline auto Heap<KeyType, ObjType, maxHeap>::GetSize() const -> size_t
{
	return m_size;
}

template <typename KeyType, typename ObjType, bool maxHeap>
inline auto Heap<KeyType, ObjType, maxHeap>::GetData() const -> pair_type*
{
	return m_data;
}

template <typename KeyType, typename ObjType, bool maxHeap>
inline auto Heap<KeyType, ObjType, maxHeap>::GetCapacity() const -> size_t
{
	return m_capacity;
}