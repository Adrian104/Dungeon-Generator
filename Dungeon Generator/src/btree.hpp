#pragma once
#include "macros.hpp"

template <typename Type>
class BTNode
{
	Type data;
	BTNode *const parent;

	BTNode *left;
	BTNode *right;

	public:
	BTNode(BTNode *const pParent, const Type &ref = Type());
	~BTNode();

	inline Type *Up() { return parent != nullptr ? &parent -> data : nullptr; }
	inline Type *Left() { return left != nullptr ? &left -> data : nullptr; }
	inline Type *Right() { return right != nullptr ? &right -> data : nullptr; }

	inline bool IsRoot() const { return parent == nullptr; }
	inline bool IsLast() const { return left == nullptr && right == nullptr; }

	template <typename T> friend class BinTree;
};

template <typename Type>
struct ExeInfo
{
	int depth;
	void *resource;
	BTNode<Type> &node;

	ExeInfo(const int d, void *res, BTNode<Type> &crr) : depth(d), resource(res), node(crr) {}
};

template <typename Type>
struct ExeHelper
{
	bool rev;
	void *resource;
	mutable int depth;
	bool (*chkFunc)(const ExeInfo<Type> &info);

	ExeHelper(bool pReverse, int pDepth, bool (*pFunc)(const ExeInfo<Type> &info), void *res = nullptr) : rev(pReverse), depth(pDepth), chkFunc(pFunc), resource(res) {}
};

template <typename Type>
class BinTree
{
	public:
	enum : uint8_t { LEFT = 0b1, RIGHT = 0b10 };

	private:
	BTNode<Type> *crr;
	BTNode<Type> *root;

	public:
	BinTree();
	~BinTree();

	bool GoUp();
	bool GoLeft();
	bool GoRight();

	void Clear();
	void DeleteNodes() const;
	void AddNodes(const Type &ref = Type());
	uint8_t AddNodes(uint8_t sides, const Type &ref = Type());

	template <typename FP, typename ...Args>
	void Execute(const ExeHelper<Type> &helper, FP *fPtr, Args ...args);

	template <typename FP, typename OP, typename ...Args>
	void ExecuteObj(const ExeHelper<Type> &helper, FP OP::*fPtr, OP *oPtr, Args ...args);

	inline Type &Get() const { return crr -> data; }
	inline void ToRoot() { crr = root; }
	inline bool IsRoot() const { return crr == root; }

	inline Type *Up() { return crr -> Up(); }
	inline Type *Left() { return crr -> Left(); }
	inline Type *Right() { return crr -> Right(); }
};

template <typename Type>
BTNode<Type>::BTNode(BTNode *const pParent, const Type &ref) : data(ref), parent(pParent), left(nullptr), right(nullptr) {}

template <typename Type>
BTNode<Type>::~BTNode()
{
	delete right;
	delete left;
}

template <typename Type>
BinTree<Type>::BinTree() : crr(nullptr), root(nullptr) { Clear(); }

template <typename Type>
BinTree<Type>::~BinTree() { delete root; }

template <typename Type>
bool BinTree<Type>::GoUp()
{
	if (crr -> parent == nullptr) return false;

	crr = crr -> parent;
	return true;
}

template <typename Type>
bool BinTree<Type>::GoLeft()
{
	if (crr -> left == nullptr) return false;

	crr = crr -> left;
	return true;
}

template <typename Type>
bool BinTree<Type>::GoRight()
{
	if (crr -> right == nullptr) return false;

	crr = crr -> right;
	return true;
}

template <typename Type>
void BinTree<Type>::Clear()
{
	delete root;
	root = new BTNode<Type>(nullptr);
	crr = root;
}

template <typename Type>
void BinTree<Type>::DeleteNodes() const
{
	delete crr -> right;
	crr -> right = nullptr;

	delete crr -> left;
	crr -> left = nullptr;
}

template <typename Type>
void BinTree<Type>::AddNodes(const Type &ref)
{
	if (crr -> left == nullptr) crr -> left = new BTNode<Type>(crr, ref);
	if (crr -> right == nullptr) crr -> right = new BTNode<Type>(crr, ref);
}

template <typename Type>
uint8_t BinTree<Type>::AddNodes(uint8_t sides, const Type &ref)
{
	if (sides & LEFT)
	{
		if (crr -> left == nullptr) crr -> left = new BTNode<Type>(crr, ref);
		else sides &= ~LEFT;
	}

	if (sides & RIGHT)
	{
		if (crr -> right == nullptr) crr -> right = new BTNode<Type>(crr, ref);
		else sides &= ~RIGHT;
	}

	return sides;
}

template <typename Type> template <typename FP, typename ...Args>
void BinTree<Type>::Execute(const ExeHelper<Type> &helper, FP *fPtr, Args ...args)
{
	if (!helper.rev)
	{
		if ((*helper.chkFunc)(ExeInfo<Type>(helper.depth, helper.resource, *crr)))
		{
			BTNode<Type> *const temp = crr;
			(*fPtr)(args...);
			crr = temp;
		}
	}

	helper.depth--;
	if (GoLeft())
	{
		Execute(helper, fPtr, args...);
		GoUp();
	}

	if (GoRight())
	{
		Execute(helper, fPtr, args...);
		GoUp();
	}

	helper.depth++;
	if (helper.rev)
	{
		if ((*helper.chkFunc)(ExeInfo<Type>(helper.depth, helper.resource, *crr)))
		{
			BTNode<Type> *const temp = crr;
			(*fPtr)(args...);
			crr = temp;
		}
	}
}

template <typename Type> template <typename FP, typename OP, typename ...Args>
void BinTree<Type>::ExecuteObj(const ExeHelper<Type> &helper, FP OP::*fPtr, OP *oPtr, Args ...args)
{
	if (!helper.rev)
	{
		if ((*helper.chkFunc)(ExeInfo<Type>(helper.depth, helper.resource, *crr)))
		{
			BTNode<Type> *const temp = crr;
			(oPtr ->* fPtr)(args...);
			crr = temp;
		}
	}

	helper.depth--;
	if (GoLeft())
	{
		ExecuteObj(helper, fPtr, oPtr, args...);
		GoUp();
	}

	if (GoRight())
	{
		ExecuteObj(helper, fPtr, oPtr, args...);
		GoUp();
	}

	helper.depth++;
	if (helper.rev)
	{
		if ((*helper.chkFunc)(ExeInfo<Type>(helper.depth, helper.resource, *crr)))
		{
			BTNode<Type> *const temp = crr;
			(oPtr ->* fPtr)(args...);
			crr = temp;
		}
	}
}