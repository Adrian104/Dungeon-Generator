#pragma once
#include "utils.hpp"

namespace bt
{
	enum class Trav : byte { PREORDER, POSTORDER };

	template <typename Type>
	struct Info;

	template <typename Type>
	class Node
	{
		struct ExeIter
		{
			Caller<void, Node<Type>&> *const caller;
			bool (*chkFunc)(const Info<Type> &info);

			ExeIter(Caller<void, Node<Type>&> *pCaller, bool (*pChkFunc)(const Info<Type> &info)) : caller(pCaller), chkFunc(pChkFunc) {}

			void Preorder(Node<Type> *node, const int counter);
			void Postorder(Node<Type> *node, const int counter);
		};

		public:
		Type data;

		Node<Type> *left;
		Node<Type> *right;
		Node<Type> *parent;

		Node(Node<Type> *const pParent) : data(Type()), left(nullptr), right(nullptr), parent(pParent) {}
		Node(Node<Type> *const pParent, const Type &ref) : data(ref), left(nullptr), right(nullptr), parent(pParent) {}
		~Node() { delete right; delete left; }

		template <typename FType>
		void Execute(Trav type, FType func, bool (*chkFunc)(const Info<Type> &info), int counter = 0);

		template <typename FType, typename Class>
		void Execute(Trav type, FType func, Class *obj, bool (*chkFunc)(const Info<Type> &info), int counter = 0);
	};

	template <typename Type>
	struct Info
	{
		const int counter;
		Node<Type> *const node;

		Info(const int pCounter, Node<Type> *const pNode) : counter(pCounter), node(pNode) {}

		inline bool IsRoot() const { return node -> parent == nullptr; }
		inline bool IsLeaf() const { return node -> left == nullptr && node -> right == nullptr; }
		inline bool IsInternal() const { return node -> left != nullptr || node -> right != nullptr; }
	};

	template <typename Type>
	void Node<Type>::ExeIter::Preorder(Node<Type> *node, const int counter)
	{
		if ((*chkFunc)(Info<Type>(counter, node))) caller -> Call(*node);

		const int nextVal = counter - 1;
		if (node -> left != nullptr) Preorder(node -> left, nextVal);
		if (node -> right != nullptr) Preorder(node -> right, nextVal);
	}

	template <typename Type>
	void Node<Type>::ExeIter::Postorder(Node<Type> *node, const int counter)
	{
		const int nextVal = counter - 1;
		if (node -> left != nullptr) Postorder(node -> left, nextVal);
		if (node -> right != nullptr) Postorder(node -> right, nextVal);

		if ((*chkFunc)(Info<Type>(counter, node))) caller -> Call(*node);
	}

	template <typename Type> template <typename FType>
	void Node<Type>::Execute(Trav type, FType func, bool (*chkFunc)(const Info<Type> &info), int counter)
	{
		FCaller<void, Node<Type>&> caller(func);
		ExeIter exeIter(&caller, chkFunc);

		if (type == Trav::PREORDER) exeIter.Preorder(this, counter);
		else exeIter.Postorder(this, counter);
	}

	template <typename Type> template <typename FType, typename Class>
	void Node<Type>::Execute(Trav type, FType func, Class *obj, bool (*chkFunc)(const Info<Type> &info), int counter)
	{
		MCaller<void, Class, Node<Type>&> caller(func, obj);
		ExeIter exeIter(&caller, chkFunc);

		if (type == Trav::PREORDER) exeIter.Preorder(this, counter);
		else exeIter.Postorder(this, counter);
	}
}