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
			int m_state = 0;

			void GoUp();
			void GoLeft();
			void GoRight();
			void Advance();

		public:
			int m_counter = 0;

			Iterator() = default;
			Iterator(pointer ptr);

			pointer operator->() { return m_crr; }
			reference operator*() const { return *m_crr; }

			auto operator++() -> Iterator&;
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
	void Node<Type>::Iterator<breakAt>::GoUp()
	{
		pointer const parent = m_crr->m_parent;
		if (parent != nullptr)
		{
			m_state = (parent->m_right == m_crr) + 1;
			m_crr = parent;
			m_counter--;
		}
		else
		{
			m_crr = nullptr;
			m_state = breakAt;
		}
	}

	template <typename Type> template <int breakAt>
	void Node<Type>::Iterator<breakAt>::GoLeft()
	{
		pointer const left = m_crr->m_left;
		if (left != nullptr)
		{
			m_crr = left;
			m_counter++;
		}
		else m_state = 1;
	}

	template <typename Type> template <int breakAt>
	void Node<Type>::Iterator<breakAt>::GoRight()
	{
		pointer const right = m_crr->m_right;
		if (right != nullptr)
		{
			m_crr = right;
			m_state = 0;
			m_counter++;
		}
		else m_state = 2;
	}

	template <typename Type> template <int breakAt>
	void Node<Type>::Iterator<breakAt>::Advance()
	{
		using scope = Node<Type>::Iterator<breakAt>;
		using action = void (scope::*)();

		static constexpr action s_actions[3] = { &scope::GoLeft, &scope::GoRight, &scope::GoUp };
		(this->*s_actions[m_state])();
	}

	template <typename Type> template <int breakAt>
	Node<Type>::Iterator<breakAt>::Iterator(pointer ptr) : m_crr(ptr)
	{
		if (m_crr == nullptr)
			return;

		while (m_state != breakAt)
			Advance();
	}

	template <typename Type> template <int breakAt>
	auto Node<Type>::Iterator<breakAt>::operator++() -> Iterator&
	{
		do { Advance(); }
		while (m_state != breakAt);

		return *this;
	}
}