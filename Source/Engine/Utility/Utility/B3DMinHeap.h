//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Utility/B3DTArray.h"

namespace b3d
{
	/** @addtogroup DataStructures
	 *  @{
	 */

	/** Nodes for the heap. */
	template <class K, class V>
	struct HeapNode
	{
		K Key;
		V Value;
		u32 Index;
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

		MinHeap(u32 elements)
		{
			Resize(elements);
		}

		MinHeap<K, V>& operator=(const MinHeap<K, V>& other)
		{
			mSize = other.mSize;
			mNode = other.mNode;
			mPtr.Resize(other.mPtr.Size());

			for(auto& entry : mNode)
				mPtr[entry.Index] = &entry;

			return *this;
		}

		HeapNode<K, V> operator[](u32 index) { return mNode[index]; }

		const HeapNode<K, V> operator[](u32 index) const { return mNode[index]; }

		bool Empty() const { return mSize == 0; }

		u32 Size() const { return mSize; }

		void Minimum(K& key, V& value)
		{
			B3D_ASSERT(mSize > 0);

			key = mPtr[0]->Key;
			value = mPtr[0]->Value;
		}

		HeapNode<K, V>* Insert(const K& key, const V& value)
		{
			if(mSize == mNode.Size())
				return nullptr;

			int child = mSize++;
			HeapNode<K, V>* node = mPtr[child];

			node->Key = key;
			node->Value = value;

			while(child > 0)
			{
				const int parent = (child - 1) / 2;

				if(mPtr[parent]->Value <= value)
					break;

				mPtr[child] = mPtr[parent];
				mPtr[child]->Index = child;

				mPtr[parent] = node;
				mPtr[parent]->Index = parent;

				child = parent;
			}

			return mPtr[child];
		}

		void Erase(K& key, V& value)
		{
			B3D_ASSERT(mSize > 0);

			HeapNode<K, V>* root = mPtr[0];
			key = root->Key;
			value = root->Value;

			const int last = --mSize;
			HeapNode<K, V>* node = mPtr[last];

			int parent = 0;
			int child = 1;

			while(child <= last)
			{
				if(child < last)
				{
					const int child2 = child + 1;

					if(mPtr[child2]->Value < mPtr[child]->Value)
						child = child2;
				}

				if(node->Value <= mPtr[child]->Value)
					break;

				mPtr[parent] = mPtr[child];
				mPtr[parent]->Index = parent;

				parent = child;
				child = 2 * child + 1;
			}

			mPtr[parent] = node;
			mPtr[parent]->Index = parent;

			mPtr[last] = root;
			mPtr[last]->Index = last;
		}

		void Update(HeapNode<K, V>* node, const V& value)
		{
			if(!node)
				return;

			int parent = 0;
			int child = 0;
			int child2 = 0;
			int maxChild = 0;

			if(node->Value < value)
			{
				node->Value = value;
				parent = node->Index;
				child = 2 * parent + 1;

				while(child < mSize)
				{
					child2 = child + 1;
					if(child2 < mSize)
					{
						if(mPtr[child]->Value <= mPtr[child2]->Value)
							maxChild = child;
						else
							maxChild = child2;
					}
					else
						maxChild = child;

					if(value <= mPtr[maxChild]->Value)
						break;

					mPtr[parent] = mPtr[maxChild];
					mPtr[parent]->Index = parent;

					mPtr[maxChild] = node;
					mPtr[maxChild]->Index = maxChild;

					parent = maxChild;
					child = 2 * parent + 1;
				}
			}
			else if(value < node->Value)
			{
				node->Value = value;
				child = node->Index;

				while(child > 0)
				{
					parent = (child - 1) / 2;

					if(mPtr[parent]->Value <= value)
						break;

					mPtr[child] = mPtr[parent];
					mPtr[child]->Index = child;

					mPtr[parent] = node;
					mPtr[parent]->Index = parent;

					child = parent;
				}
			}
		}

		void Resize(u32 elements)
		{
			mSize = 0;
			if(elements > 0)
			{
				mNode.Resize(elements);
				mPtr.Resize(elements);

				for(u32 elementIndex = 0; elementIndex < elements; ++elementIndex)
				{
					mPtr[elementIndex] = &mNode[elementIndex];
					mPtr[elementIndex]->Index = elementIndex;
				}
			}
			else
			{
				mNode.Clear();
				mPtr.Clear();
			}
		}

		bool Valid() const
		{
			for(int nodeIndex = 0; nodeIndex < (int)mSize; ++nodeIndex)
			{
				int parent = (nodeIndex - 1) / 2;
				if(parent > 0)
				{
					if(mPtr[nodeIndex]->Value < mPtr[parent]->Value ||
					   (int)mPtr[parent]->Index != parent)
						return false;
				}
			}

			return true;
		}

	private:
		u32 mSize;
		TArray<HeapNode<K, V>> mNode;
		TArray<HeapNode<K, V>*> mPtr;
	};

	/** @} */
} // namespace b3d
