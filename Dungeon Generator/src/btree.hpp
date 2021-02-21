#pragma once

typedef unsigned char uchar;

template <typename Type>
class Node
{
	Type data;
	Node *const parent;

	Node *left;
	Node *right;

	public:
	Node(Node *const pParent, const Type &ref = Type());
	~Node();

	inline bool IsRoot() const { return parent == nullptr; }
	inline bool IsLast() const { return left == nullptr && right == nullptr; }

	template <typename T> friend class BTree;
};

template <typename Type>
struct ExeHelper
{
	bool rev;
	int depth;
	bool (*chkFunc)(int, Node<Type>*);

	ExeHelper(bool pReverse, int pDepth, bool (*pFunc)(int, Node<Type>*)) : rev(pReverse), depth(pDepth), chkFunc(pFunc) {}
};

template <typename Type>
class BTree
{
	public:
	enum : uchar { LEFT = 0b1, RIGHT = 0b10 };

	private:
	Node<Type> root;
	Node<Type> *crr;

	public:
	BTree();

	bool GoUp();
	bool GoLeft();
	bool GoRight();

	void DeleteNode() const;
	uchar AddNode(const Type &ref = Type(), uchar sides = LEFT | RIGHT);

	template <typename FP, typename ...Args>
	void Execute(ExeHelper<Type> helper, FP *fPtr, Args ...args);

	template <typename FP, typename OP, typename ...Args>
	void ExecuteObj(ExeHelper<Type> helper, FP OP::*fPtr, OP *oPtr, Args ...args);

	inline Type &Get() const { return crr -> data; }
	inline void ToRoot() { crr = &root; }
	inline bool IsRoot() const { return crr == &root; }
};

template <typename Type>
Node<Type>::Node(Node *const pParent, const Type &ref) : data(ref), parent(pParent), left(nullptr), right(nullptr) {}

template <typename Type>
Node<Type>::~Node()
{
	delete right;
	delete left;
}

template <typename Type>
BTree<Type>::BTree() : root(nullptr), crr(&root) {}

template <typename Type>
bool BTree<Type>::GoUp()
{
	if (crr -> parent == nullptr) return false;

	crr = crr -> parent;
	return true;
}

template <typename Type>
bool BTree<Type>::GoLeft()
{
	if (crr -> left == nullptr) return false;

	crr = crr -> left;
	return true;
}

template <typename Type>
bool BTree<Type>::GoRight()
{
	if (crr -> right == nullptr) return false;

	crr = crr -> right;
	return true;
}

template <typename Type>
void BTree<Type>::DeleteNode() const
{
	delete crr -> right;
	crr -> right = nullptr;

	delete crr -> left;
	crr -> left = nullptr;
}

template <typename Type>
uchar BTree<Type>::AddNode(const Type &ref, uchar sides)
{
	if (sides & LEFT)
	{
		if (crr -> left == nullptr) crr -> left = new Node<Type>(crr, ref);
		else sides &= ~LEFT;
	}

	if (sides & RIGHT)
	{
		if (crr -> right == nullptr) crr -> right = new Node<Type>(crr, ref);
		else sides &= ~RIGHT;
	}

	return sides;
}

template <typename Type> template <typename FP, typename ...Args>
void BTree<Type>::Execute(ExeHelper<Type> helper, FP *fPtr, Args ...args)
{
	if (!helper.rev)
	{
		if ((*helper.chkFunc)(helper.depth, crr))
		{
			Node<Type> *const temp = crr;
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

	if (helper.rev)
	{
		helper.depth++;
		if ((*helper.chkFunc)(helper.depth, crr))
		{
			Node<Type> *const temp = crr;
			(*fPtr)(args...);
			crr = temp;
		}
	}
}

template <typename Type> template <typename FP, typename OP, typename ...Args>
void BTree<Type>::ExecuteObj(ExeHelper<Type> helper, FP OP::*fPtr, OP *oPtr, Args ...args)
{
	if (!helper.rev)
	{
		if ((*helper.chkFunc)(helper.depth, crr))
		{
			Node<Type> *const temp = crr;
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

	if (helper.rev)
	{
		helper.depth++;
		if ((*helper.chkFunc)(helper.depth, crr))
		{
			Node<Type> *const temp = crr;
			(oPtr ->* fPtr)(args...);
			crr = temp;
		}
	}
}