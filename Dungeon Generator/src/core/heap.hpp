#pragma once
#include "pch.hpp"

template <typename ValType, typename ObjType>
class Heap
{
	struct Pair
	{
		ValType value;
		ObjType object;
	};

	Pair *arr;
	size_t size, capacity;

	void Expand();

	public:
	Heap();
	~Heap();

	void Clear();
	void Reset();

	ObjType Pop();
	void Push(ValType value, const ObjType &obj);
};

template <typename ValType, typename ObjType>
void Heap<ValType, ObjType>::Expand()
{
	capacity <<= 1;

	Pair *const newArr = (Pair*)operator new(sizeof(Pair) * capacity);
	Pair *const arrEndIter = arr + size;
	Pair *newArrIter = newArr;
	Pair *arrIter = arr;

	while (arrIter != arrEndIter)
	{
		*newArrIter = *arrIter;

		arrIter++;
		newArrIter++;
	}

	operator delete[](arr);
	arr = newArr;
}

template <typename ValType, typename ObjType>
Heap<ValType, ObjType>::Heap() : size(0), capacity(1) { arr = (Pair*)operator new(sizeof(Pair)); }

template <typename ValType, typename ObjType>
Heap<ValType, ObjType>::~Heap() { operator delete[](arr); }

template <typename ValType, typename ObjType>
void Heap<ValType, ObjType>::Clear() { size = 0; }

template <typename ValType, typename ObjType>
void Heap<ValType, ObjType>::Reset()
{
	size = 0;
	capacity = 1;

	operator delete[](arr);
	arr = (Pair*)operator new(sizeof(Pair));
}

template <typename ValType, typename ObjType>
ObjType Heap<ValType, ObjType>::Pop()
{
	if (size == 0) throw std::runtime_error("Calling Pop() on empty heap");

	const ObjType ret = arr -> object;
	if (--size == 0) return ret;

	*arr = arr[size];
	size_t crrIndex = 0;

	while (true)
	{
		size_t minIndex = (crrIndex << 1) + 1;
		if (const size_t rIndex = minIndex + 1; rIndex < size)
		{
			if (arr[rIndex].value < arr[minIndex].value) minIndex = rIndex;
		}
		else if (minIndex >= size) break;

		Pair &crrPair = arr[crrIndex];
		Pair &minPair = arr[minIndex];

		if (crrPair.value <= minPair.value) break;
		crrIndex = minIndex;

		const Pair tmpPair = crrPair;

		crrPair = minPair;
		minPair = tmpPair;
	}

	return ret;
}

template <typename ValType, typename ObjType>
void Heap<ValType, ObjType>::Push(ValType value, const ObjType &obj)
{
	if (size >= capacity) Expand();

	arr[size] = { value, obj };
	size_t crrIndex = size;

	size++;
	while (crrIndex > 0)
	{
		const size_t parIndex = (crrIndex - 1) >> 1;

		Pair &parPair = arr[parIndex];
		Pair &crrPair = arr[crrIndex];

		if (parPair.value <= crrPair.value) break;
		crrIndex = parIndex;

		const Pair tmpPair = crrPair;

		crrPair = parPair;
		parPair = tmpPair;
	}
}