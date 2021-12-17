#pragma once
#include "pch.hpp"

template <typename KeyType, typename ObjType>
class Heap
{
	typedef std::pair<KeyType, ObjType> pair_type;

	size_t m_size = 0;
	size_t m_capacity = 0;
	pair_type* m_data = nullptr;

	void Expand();

	public:
	Heap();
	~Heap();

	Heap(const Heap& ref);
	auto operator=(const Heap& ref) -> Heap&;

	Heap(Heap&& ref) noexcept;
	auto operator=(Heap&& ref) noexcept -> Heap&;

	void Pop();
	void Clear(bool reset = false);
	void Push(KeyType key, const ObjType& object);

	ObjType& Top() const;
	size_t GetSize() const;
	size_t GetCapacity() const;
};

template <typename KeyType, typename ObjType>
void Heap<KeyType, ObjType>::Expand()
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

template <typename KeyType, typename ObjType>
inline Heap<KeyType, ObjType>::Heap() {}

template <typename KeyType, typename ObjType>
inline Heap<KeyType, ObjType>::~Heap() { Clear(true); }

template <typename KeyType, typename ObjType>
Heap<KeyType, ObjType>::Heap(const Heap& ref) : m_size(ref.m_size), m_capacity(ref.m_capacity)
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

template <typename KeyType, typename ObjType>
auto Heap<KeyType, ObjType>::operator=(const Heap& ref) -> Heap&
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

template <typename KeyType, typename ObjType>
Heap<KeyType, ObjType>::Heap(Heap&& ref) noexcept : m_size(ref.m_size), m_capacity(ref.m_capacity), m_data(ref.m_data)
{
	ref.m_size = 0;
	ref.m_capacity = 0;
	ref.m_data = nullptr;
}

template <typename KeyType, typename ObjType>
auto Heap<KeyType, ObjType>::operator=(Heap&& ref) noexcept -> Heap&
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

template <typename KeyType, typename ObjType>
void Heap<KeyType, ObjType>::Pop()
{
	if (m_size == 0) throw std::runtime_error("Calling Pop() on empty heap");

	m_data -> ~pair_type();
	if (--m_size == 0) return;

	*m_data = std::move(m_data[m_size]);
	size_t crrIndex = 0;

	while (true)
	{
		size_t minIndex = (crrIndex << 1) + 1;
		if (const size_t rIndex = minIndex + 1; rIndex < m_size)
		{
			if (m_data[rIndex].first < m_data[minIndex].first) minIndex = rIndex;
		}
		else if (minIndex >= m_size) break;

		pair_type& crrPair = m_data[crrIndex];
		pair_type& minPair = m_data[minIndex];

		if (crrPair.first <= minPair.first) break;

		crrIndex = minIndex;
		std::swap(crrPair, minPair);
	}
}

template <typename KeyType, typename ObjType>
void Heap<KeyType, ObjType>::Clear(bool reset)
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

template <typename KeyType, typename ObjType>
void Heap<KeyType, ObjType>::Push(KeyType key, const ObjType& object)
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

		if (parPair.first <= crrPair.first) break;

		crrIndex = parIndex;
		std::swap(crrPair, parPair);
	}
}

template <typename KeyType, typename ObjType>
inline ObjType& Heap<KeyType, ObjType>::Top() const
{
	return m_data -> second;
}

template <typename KeyType, typename ObjType>
inline size_t Heap<KeyType, ObjType>::GetSize() const
{
	return m_size;
}

template <typename KeyType, typename ObjType>
inline size_t Heap<KeyType, ObjType>::GetCapacity() const
{
	return m_capacity;
}