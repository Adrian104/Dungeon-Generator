// SPDX-FileCopyrightText: Copyright (c) 2023 Adrian Kulawik
// SPDX-License-Identifier: MIT

#pragma once
#include <iterator>
#include <utility>

namespace dg::impl
{
	template <typename Type>
	struct Node : public Type
	{
		template <int breakAt>
		struct Iterator
		{
			using iterator_category = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = Node<Type>;
			using pointer = value_type*;
			using reference = value_type&;

		private:
			pointer m_crr = nullptr;
			void Advance(int state);

		public:
			int m_counter = 0;

			Iterator() = default;
			Iterator(pointer ptr) : m_crr(ptr) { if (m_crr != nullptr && breakAt != 0) Advance(0); }

			pointer operator->() { return m_crr; }
			reference operator*() const { return *m_crr; }

			auto operator++() -> Iterator& { Advance(breakAt); return *this; }
			Iterator operator++(int) { Iterator iter = *this; ++(*this); return iter; }

			bool operator==(const Iterator& iter) const { return m_crr == iter.m_crr; }
			bool operator!=(const Iterator& iter) const { return m_crr != iter.m_crr; }
		};

		template <int breakAt>
		struct Range
		{
			Node* const m_ptr;
			Range(Node* ptr) : m_ptr(ptr) {}

			Iterator<breakAt> begin() { return Iterator<breakAt>(m_ptr); }
			Iterator<breakAt> end() { return Iterator<breakAt>(); }
		};

		Node* m_parent;
		Node* m_left = nullptr;
		Node* m_right = nullptr;

		Node(Node* parent, Type&& ref) : Type(std::move(ref)), m_parent(parent) {}
		Node(Node* parent, const Type& ref) : Type(ref), m_parent(parent) {}
		~Node() = default;

		Node(const Node& ref) = delete;
		Node& operator=(const Node& ref) = delete;

		Node(Node&& ref) noexcept = delete;
		Node& operator=(Node&& ref) noexcept = delete;

		Range<0> Preorder() { return Range<0>(this); }
		Range<1> Inorder() { return Range<1>(this); }
		Range<2> Postorder() { return Range<2>(this); }
	};

	template <typename Type> template <int breakAt>
	void Node<Type>::Iterator<breakAt>::Advance(int state)
	{
		pointer temp;
		while (true)
		{
			switch (state)
			{
			case 0:
				if (temp = m_crr->m_left; temp != nullptr)
				{
					m_crr = temp;
					m_counter++;

					if constexpr (breakAt == 0)
						return;
					else
						continue;
				}

				if constexpr (breakAt == 1)
					return;
				[[fallthrough]];

			case 1:
				if (temp = m_crr->m_right; temp != nullptr)
				{
					m_crr = temp;
					m_counter++;

					if constexpr (breakAt == 0)
						return;
					else
					{
						state = 0;
						continue;
					}
				}

				if constexpr (breakAt == 2)
					return;
				[[fallthrough]];

			default:
				temp = m_crr;
				m_crr = m_crr->m_parent;
				m_counter--;

				if (m_crr == nullptr)
					return;

				if (m_crr->m_right == temp)
				{
					if constexpr (breakAt == 2)
						return;
					else
					{
						state = 2;
						continue;
					}
				}
				else
				{
					if constexpr (breakAt == 1)
						return;
					else
					{
						state = 1;
						continue;
					}
				}
			}
		}
	}
}