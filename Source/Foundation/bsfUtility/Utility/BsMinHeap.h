#pragma once

#include "Utility/BsDynArray.h"

namespace bs
{

	/** Nodes for the heap. */
	template <class K, class V>
	struct HeapNode
	{
		K key;
		V value;
		UINT32 index;
	};

	/**
	* Binary tree where each nodes is less than or equal to the data in its node's children.
	*/
	template <class K, class V>
	class MinHeap
	{
	public:
		MinHeap() = default;

		MinHeap(const MinHeap<K, V>& other)
		{
			*this = other;
		}

		MinHeap(UINT32 elements)
		{
			resize(elements);
		}

		MinHeap<K, V>& operator= (const MinHeap<K, V>& other)
		{
			mSize = other.mSize;
			mNode = other.mNode;
			mPtr.resize(other.mPtr.size());

			for (auto& entry : mNode)
				mPtr[entry.index] = &entry;

			return *this;
		}

		HeapNode<K, V> operator[] (UINT32 index) { return mNode[index]; }
		const HeapNode<K, V> operator[] (UINT32 index) const { return mNode[index]; }

		bool empty() const { return mSize == 0; }
		UINT32 size() const { return mSize; }

		void minimum(K& key, V& value)
		{
			assert(mSize > 0);

			key = mPtr[0]->key;
			value = mPtr[0]->value;
		}

		HeapNode<K, V>* insert(const K& key, const V& value)
		{
			if (mSize == mNode.size())
				return nullptr;

			int child = mSize++;
			HeapNode<K, V>* node = mPtr[child];

			node->key = key;
			node->value = value;

			while (child > 0)
			{
				const int parent = (child - 1) / 2;

				if (mPtr[parent]->value <= value)
					break;

				mPtr[child] = mPtr[parent];
				mPtr[child]->index = child;

				mPtr[parent] = node;
				mPtr[parent]->index = parent;

				child = parent;
			}

			return mPtr[child];
		}

		void erase(K& key, V& value)
		{
			assert(mSize > 0);

			HeapNode<K, V>* root = mPtr[0];
			key = root->key;
			value = root->value;

			const int last = --mSize;
			HeapNode<K, V>* node = mPtr[last];

			int parent = 0;
			int child = 1;

			while (child <= last)
			{
				if (child < last)
				{
					const int child2 = child + 1;

					if (mPtr[child2]->value < mPtr[child]->value)
						child = child2;
				}

				if (node->value <= mPtr[child]->value)
					break;

				mPtr[parent] = mPtr[child];
				mPtr[parent]->index = parent;

				parent = child;
				child = 2 * child + 1;
			}

			mPtr[parent] = node;
			mPtr[parent]->index = parent;

			mPtr[last] = root;
			mPtr[last]->index = last;
		}

		void update(HeapNode<K, V>* node, const V& value)
		{
			if (!node)
				return;

			int parent = 0;
			int child = 0;
			int child2 = 0;
			int maxChild = 0;

			if (node->value < value)
			{
				node->value = value;
				parent = node->index;
				child = 2 * parent + 1;

				while (child < mSize)
				{
					child2 = child + 1;
					if (child2 < mSize)
					{
						if (mPtr[child]->value <= mPtr[child2]->value)
							maxChild = child;
						else
							maxChild = child2;
					}
					else
						maxChild = child;

					if (value <= mPtr[maxChild]->value)
						break;

					mPtr[parent] = mPtr[maxChild];
					mPtr[parent]->index = parent;

					mPtr[maxChild] = node;
					mPtr[maxChild]->index = maxChild;

					parent = maxChild;
					child = 2 * parent + 1;
				}
			}
			else if (value < node->value)
			{
				node->value = value;
				child = node->index;

				while (child > 0)
				{
					parent = (child - 1) / 2;

					if (mPtr[parent]->value <= value)
						break;

					mPtr[child] = mPtr[parent];
					mPtr[child]->index = child;

					mPtr[parent] = node;
					mPtr[parent]->index = parent;

					child = parent;
				}
			}
		}

		void resize(UINT32 elements)
		{
			mSize = 0;
			if (elements > 0)
			{
				mNode.resize(elements);
				mPtr.resize(elements);

				for (UINT32 i = 0; i < elements; ++i)
				{
					mPtr[i] = &mNode[i];
					mPtr[i]->index = i;
				}
			}
			else
			{
				mNode.clear();
				mPtr.clear();
			}
		}

		bool valid() const
		{
			for (int i = 0; i < (int)mSize; ++i)
			{
				int parent = (i - 1) / 2;
				if (parent > 0)
				{
					if (mPtr[i]->value < mPtr[parent]->value ||
						(int)mPtr[parent]->index != parent)
						return false;
				}
			}

			return true;
		}

	private:
		UINT32 mSize;
		DynArray<HeapNode<K, V>> mNode;
		DynArray<HeapNode<K, V>*> mPtr;
	};
}