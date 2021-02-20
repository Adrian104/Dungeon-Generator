#pragma once

typedef unsigned char uchar;

template <typename Type>
class Node
{
	Type data;
	Node *const parent;

	Node *left;
	Node *right;

	Node(Node *const pParent, const Type &ref = Type());
	~Node();

	template <typename T> friend class BTree;
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
	void Execute(int depth, FP *fPtr, Args ...args);

	template <typename FP, typename OP, typename ...Args>
	void ExecuteObj(int depth, FP OP::*fPtr, OP *oPtr, Args ...args);

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
void BTree<Type>::Execute(int depth, FP *fPtr, Args ...args)
{
	if (depth <= 0)
	{
		Node<Type> *const temp = crr;
		(*fPtr)(args...);
		crr = temp;

		if (depth == 0) return;
	}
	else depth--;

	if (GoLeft())
	{
		Execute(depth, fPtr, args...);
		GoUp();
	}

	if (GoRight())
	{
		Execute(depth, fPtr, args...);
		GoUp();
	}
}

template <typename Type> template <typename FP, typename OP, typename ...Args>
void BTree<Type>::ExecuteObj(int depth, FP OP::*fPtr, OP *oPtr, Args ...args)
{
	if (depth <= 0)
	{
		Node<Type> *const temp = crr;
		(oPtr ->* fPtr)(args...);
		crr = temp;

		if (depth == 0) return;
	}
	else depth--;

	if (GoLeft())
	{
		ExecuteObj(depth, fPtr, oPtr, args...);
		GoUp();
	}

	if (GoRight())
	{
		ExecuteObj(depth, fPtr, oPtr, args...);
		GoUp();
	}
}