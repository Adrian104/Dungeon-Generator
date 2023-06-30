#pragma once
#include <iterator>
#include <utility>

namespace bt
{
	using state_type = unsigned int;
	enum class Traversal : state_type { PREORDER, INORDER, POSTORDER };

	template <typename Type>
	class Node : public Type
	{
		class Iterator
		{
			public:
			using iterator_category = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = Node<Type>;
			using pointer = value_type*;
			using reference = value_type&;

			private:
			pointer m_crr;
			state_type m_breakAt;
			state_type m_state = 0;

			void GoUp();
			void GoLeft();
			void GoRight();
			void Advance();

			public:
			int m_counter = 0;

			Iterator(pointer tree, Traversal traversal);

			pointer operator->() { return m_crr; }
			reference operator*() const { return *m_crr; }

			auto operator++() -> Iterator&;
			Iterator operator++(int) { Iterator iter = *this; ++(*this); return iter; }

			bool operator==(const Iterator& iter) const { return m_crr == iter.m_crr; }
			bool operator!=(const Iterator& iter) const { return m_crr != iter.m_crr; }
		};

		public:
		static Traversal defaultTraversal;

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

		Iterator begin() { return Iterator(this, defaultTraversal); }
		Iterator end() { return Iterator(nullptr, defaultTraversal); }
	};

	template <typename Type>
	void Node<Type>::Iterator::GoUp()
	{
		pointer const parent = m_crr -> m_parent;
		if (parent != nullptr)
		{
			m_state = (parent -> m_right == m_crr) + 1;
			m_crr = parent;
			m_counter--;
		}
		else
		{
			m_crr = nullptr;
			m_state = m_breakAt;
		}
	}

	template <typename Type>
	void Node<Type>::Iterator::GoLeft()
	{
		pointer const left = m_crr -> m_left;
		if (left != nullptr)
		{
			m_crr = left;
			m_counter++;
		}
		else m_state = 1;
	}

	template <typename Type>
	void Node<Type>::Iterator::GoRight()
	{
		pointer const right = m_crr -> m_right;
		if (right != nullptr)
		{
			m_crr = right;
			m_state = 0;
			m_counter++;
		}
		else m_state = 2;
	}

	template <typename Type>
	void Node<Type>::Iterator::Advance()
	{
		using scope = Node<Type>::Iterator;
		using action = void (scope::*)();

		static constexpr action actions[3] = { &scope::GoLeft, &scope::GoRight, &scope::GoUp };
		(this ->* actions[m_state])();
	}

	template <typename Type>
	Node<Type>::Iterator::Iterator(pointer tree, Traversal traversal)
		: m_crr(tree), m_breakAt(static_cast<state_type>(traversal))
	{
		if (m_crr == nullptr)
			return;

		while (m_state != m_breakAt)
			Advance();
	}

	template <typename Type>
	auto Node<Type>::Iterator::operator++() -> Iterator&
	{
		do { Advance(); }
		while (m_state != m_breakAt);

		return *this;
	}

	template <typename Type>
	Traversal Node<Type>::defaultTraversal = Traversal::PREORDER;
}