// SPDX-FileCopyrightText: Copyright (c) 2023 Adrian Kulawik
// SPDX-License-Identifier: MIT

#pragma once
#include <algorithm>
#include <utility>
#include <stddef.h>

namespace dg::impl
{
	template <typename KeyType, typename ObjType, bool maxHeap>
	class Heap
	{
	public:
		using pair_type = std::pair<KeyType, ObjType>;

	private:
		pair_type* m_data = nullptr;
		size_t m_size = 0;
		size_t m_capacity = 0;

		void Reallocate();

	public:
		Heap() = default;
		~Heap() { Reset(); }

		Heap(const Heap& ref);
		auto operator=(const Heap& ref) -> Heap&;

		Heap(Heap&& ref) noexcept;
		auto operator=(Heap&& ref) noexcept -> Heap&;

		void Clear();
		void Reset();
		void Expand();
		void Reserve(size_t newCapacity);

		void Pop();
		void Push(KeyType key, const ObjType& object);

		size_t Size() const { return m_size; }
		size_t Capacity() const { return m_capacity; }
		pair_type* Data() const { return m_data; }
		KeyType TopKey() const { return m_data->first; }
		ObjType& TopObject() const { return m_data->second; }
	};

	template <typename KeyType, typename ObjType>
	using MinHeap = Heap<KeyType, ObjType, false>;

	template <typename KeyType, typename ObjType>
	using MaxHeap = Heap<KeyType, ObjType, true>;

	template <typename KeyType, typename ObjType, bool maxHeap>
	void Heap<KeyType, ObjType, maxHeap>::Reallocate()
	{
		pair_type* prevArr = m_data + m_size;
		pair_type* newArr = static_cast<pair_type*>(operator new[](sizeof(pair_type) * m_capacity)) + m_size;

		while (prevArr != m_data)
		{
			--prevArr;
			--newArr;

			new(newArr) pair_type(std::move(*prevArr));
			prevArr->~pair_type();
		}

		operator delete[](m_data);
		m_data = newArr;
	}

	template <typename KeyType, typename ObjType, bool maxHeap>
	Heap<KeyType, ObjType, maxHeap>::Heap(const Heap& ref) : m_size(ref.m_size), m_capacity(ref.m_size)
	{
		if (ref.m_size > 0)
		{
			pair_type* refArr = ref.m_data + ref.m_size;
			m_data = static_cast<pair_type*>(operator new[](sizeof(pair_type) * ref.m_size)) + ref.m_size;

			while (refArr != ref.m_data)
			{
				--m_data;
				--refArr;

				new(m_data) pair_type(*refArr);
			}
		}
	}

	template <typename KeyType, typename ObjType, bool maxHeap>
	auto Heap<KeyType, ObjType, maxHeap>::operator=(const Heap& ref) -> Heap&
	{
		if (&ref == this)
			return *this;

		Reset();
		if (ref.m_size > 0)
		{
			m_size = ref.m_size;
			m_capacity = ref.m_size;

			pair_type* refArr = ref.m_data + ref.m_size;
			m_data = static_cast<pair_type*>(operator new[](sizeof(pair_type) * ref.m_size)) + ref.m_size;

			while (refArr != ref.m_data)
			{
				--m_data;
				--refArr;

				new(m_data) pair_type(*refArr);
			}
		}

		return *this;
	}

	template <typename KeyType, typename ObjType, bool maxHeap>
	Heap<KeyType, ObjType, maxHeap>::Heap(Heap&& ref) noexcept
		: m_data(std::exchange(ref.m_data, nullptr)), m_size(std::exchange(ref.m_size, 0)), m_capacity(std::exchange(ref.m_capacity, 0)) {}

	template <typename KeyType, typename ObjType, bool maxHeap>
	auto Heap<KeyType, ObjType, maxHeap>::operator=(Heap&& ref) noexcept -> Heap&
	{
		if (&ref == this)
			return *this;

		Reset();

		m_data = std::exchange(ref.m_data, nullptr);
		m_size = std::exchange(ref.m_size, 0);
		m_capacity = std::exchange(ref.m_capacity, 0);

		return *this;
	}

	template <typename KeyType, typename ObjType, bool maxHeap>
	void Heap<KeyType, ObjType, maxHeap>::Clear()
	{
		pair_type* iter = m_data + m_size;
		while (iter != m_data)
		{
			--iter;
			iter->~pair_type();
		}

		m_size = 0;
	}

	template <typename KeyType, typename ObjType, bool maxHeap>
	void Heap<KeyType, ObjType, maxHeap>::Reset()
	{
		Clear();

		operator delete[](m_data);
		m_data = nullptr;
		m_capacity = 0;
	}

	template <typename KeyType, typename ObjType, bool maxHeap>
	void Heap<KeyType, ObjType, maxHeap>::Expand()
	{
		m_capacity = (m_capacity > 0) ? (m_capacity << 1) : 1;
		Reallocate();
	}

	template <typename KeyType, typename ObjType, bool maxHeap>
	void Heap<KeyType, ObjType, maxHeap>::Reserve(size_t newCapacity)
	{
		m_capacity = std::max(newCapacity, m_size);
		if (m_capacity > 0) Reallocate();
		else Reset();
	}

	template <typename KeyType, typename ObjType, bool maxHeap>
	void Heap<KeyType, ObjType, maxHeap>::Pop()
	{
		if (m_size < 2)
		{
			if (m_size > 0)
				m_data->~pair_type();

			m_size = 0;
			return;
		}

		*m_data = std::move(m_data[--m_size]);
		m_data[m_size].~pair_type();

		size_t crrIndex = 0;
		while (true)
		{
			size_t chdIndex = (crrIndex << 1) + 1;
			if (chdIndex >= m_size) break;

			if (const size_t rIndex = chdIndex + 1; rIndex < m_size)
			{
				if constexpr (maxHeap)
				{
					if (m_data[rIndex].first > m_data[chdIndex].first)
						chdIndex = rIndex;
				}
				else
				{
					if (m_data[rIndex].first < m_data[chdIndex].first)
						chdIndex = rIndex;
				}
			}

			pair_type& crrPair = m_data[crrIndex];
			pair_type& chdPair = m_data[chdIndex];

			if constexpr (maxHeap)
			{
				if (crrPair.first >= chdPair.first)
					break;
			}
			else
			{
				if (crrPair.first <= chdPair.first)
					break;
			}

			crrIndex = chdIndex;
			std::swap(crrPair, chdPair);
		}
	}

	template <typename KeyType, typename ObjType, bool maxHeap>
	void Heap<KeyType, ObjType, maxHeap>::Push(KeyType key, const ObjType& object)
	{
		if (m_size >= m_capacity)
			Expand();

		new(m_data + m_size) pair_type(key, object);
		size_t crrIndex = m_size++;

		while (crrIndex > 0)
		{
			const size_t parIndex = (crrIndex - 1) >> 1;

			pair_type& parPair = m_data[parIndex];
			pair_type& crrPair = m_data[crrIndex];

			if constexpr (maxHeap)
			{
				if (parPair.first >= crrPair.first)
					break;
			}
			else
			{
				if (parPair.first <= crrPair.first)
					break;
			}

			crrIndex = parIndex;
			std::swap(crrPair, parPair);
		}
	}
}