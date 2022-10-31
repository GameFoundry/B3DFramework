//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/BsBinaryCompare.h"
#include "Reflection/BsRTTIType.h"
#include "FileSystem/BsDataStream.h"

using namespace bs;

namespace impl
{
template <class T>
struct ScopeGuard
{
	~ScopeGuard()
	{
		Callback();
	}

	T Callback;
};

template <class T>
ScopeGuard<T> make_scope_guard(T callback)
{
	return ScopeGuard<T>{ callback };
}
} // namespace impl

BinaryCompare::BinaryCompare()
	: mAlloc(&GetFrameAllocator())
{}

bool BinaryCompare::Run(IReflectable& a, IReflectable& b)
{
	mAlloc->MarkFrame();
	bool output = Compare(a, b);
	mObjectMap.clear();
	mAlloc->Clear();

	return output;
}

bool BinaryCompare::Compare(IReflectable& a, IReflectable& b)
{
	RTTITypeBase* rtti = a.GetRtti();

	if(rtti != b.GetRtti())
		return false;

	struct RTTIPair
	{
		RTTITypeBase* RttiA;
		RTTITypeBase* RttiB;
	};

	FrameStack<RTTIPair> rttiInstances;
	auto cleanup = ::impl::make_scope_guard([&]()
										  {
			while (!rttiInstances.empty())
			{
				RTTIPair rttiPair = rttiInstances.top();
				rttiPair.RttiA->OnSerializationEnded(&a, mContext);
				rttiPair.RttiB->OnSerializationEnded(&b, mContext);
				mAlloc->Destruct(rttiPair.RttiA);
				mAlloc->Destruct(rttiPair.RttiB);

				rttiInstances.pop();
			} });

	// If an object has base classes, we need to iterate through all of them
	do
	{
		RTTITypeBase* rttiInstanceA = rtti->CloneInternal(*mAlloc);
		RTTITypeBase* rttiInstanceB = rtti->CloneInternal(*mAlloc);
		rttiInstances.push({ rttiInstanceA, rttiInstanceB });

		rttiInstanceA->OnSerializationStarted(&a, mContext);
		rttiInstanceB->OnSerializationStarted(&b, mContext);

		const u32 numFields = rtti->GetNumFields();
		for(u32 i = 0; i < numFields; i++)
		{
			RTTIField* curGenericField = rtti->GetField(i);
			if(curGenericField->Schema.IsArray)
			{
				const u32 arrayNumElemsA = curGenericField->GetArraySize(rttiInstanceA, &a);
				const u32 arrayNumElemsB = curGenericField->GetArraySize(rttiInstanceB, &b);

				if(arrayNumElemsA != arrayNumElemsB)
					return false;

				switch(curGenericField->Schema.Type)
				{
				case SerializableFT_ReflectablePtr:
					{
						auto curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);

						for(u32 arrIdx = 0; arrIdx < arrayNumElemsA; arrIdx++)
						{
							SPtr<IReflectable> childObjectA = curField->GetArrayValue(rttiInstanceA, &a, arrIdx);
							SPtr<IReflectable> childObjectB = curField->GetArrayValue(rttiInstanceB, &b, arrIdx);

							if(childObjectA != childObjectB)
							{
								if(childObjectA == nullptr || childObjectB == nullptr)
									return false;

								RTTITypeBase* childRtti = nullptr;
								if(childObjectA->GetRtti() == childObjectB->GetRtti())
									childRtti = childObjectA->GetRtti();

								if(childRtti != nullptr)
								{
									ICompare& handler = childRtti->GetCompareHandler();
									if(!handler.Run(*childObjectA, *childObjectB))
										return false;
								}
								else
									return false;
							}
						}

						break;
					}
				case SerializableFT_Reflectable:
					{
						auto curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);

						for(u32 arrIdx = 0; arrIdx < arrayNumElemsA; arrIdx++)
						{
							IReflectable& childObjectA = curField->GetArrayValue(rttiInstanceA, &a, arrIdx);
							IReflectable& childObjectB = curField->GetArrayValue(rttiInstanceB, &b, arrIdx);

							RTTITypeBase* childRtti = nullptr;
							if(childObjectA.GetRtti() == childObjectB.GetRtti())
								childRtti = childObjectA.GetRtti();

							if(childRtti != nullptr)
							{
								ICompare& handler = childRtti->GetCompareHandler();
								if(!handler.Run(childObjectA, childObjectB))
									return false;
							}
							else
								return false;
						}

						break;
					}
				case SerializableFT_Plain:
					{
						auto curField = static_cast<RTTIPlainFieldBase*>(curGenericField);

						for(u32 arrIdx = 0; arrIdx < arrayNumElemsA; arrIdx++)
						{
							u32 typeSizeA = 0;
							u32 typeSizeB = 0;
							if(curField->Schema.HasDynamicSize)
							{
								typeSizeA = curField->GetArrayElemDynamicSize(rttiInstanceA, &a, arrIdx, false).Bytes;
								typeSizeB = curField->GetArrayElemDynamicSize(rttiInstanceB, &b, arrIdx, false).Bytes;
							}
							else
								typeSizeA = typeSizeB = curField->Schema.Size.Bytes;

							if(typeSizeA != typeSizeB)
								return false;

							// Note: Ideally avoid doing copies here, and compare field values directly
							auto dataA = bs_managed_stack_alloc(typeSizeA);
							auto dataB = bs_managed_stack_alloc(typeSizeB);

							Bitstream streamA((uint8_t*)static_cast<void*>(dataA), typeSizeA);
							Bitstream streamB((uint8_t*)static_cast<void*>(dataB), typeSizeB);

							curField->ArrayElemToStream(rttiInstanceA, &a, arrIdx, streamA);
							curField->ArrayElemToStream(rttiInstanceB, &b, arrIdx, streamB);

							if(memcmp(dataA, dataB, typeSizeA) != 0)
								return false;
						}

						break;
					}
				default:
					BS_EXCEPT(InternalErrorException, "Error encoding data. Encountered a type I don't know how to encode. Type: " + toString(u32(curGenericField->Schema.Type)) + ", Is array: " + toString(curGenericField->Schema.IsArray));
				}
			}
			else
			{
				switch(curGenericField->Schema.Type)
				{
				case SerializableFT_ReflectablePtr:
					{
						auto curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);

						SPtr<IReflectable> childObjectA = curField->GetValue(rttiInstanceA, &a);
						SPtr<IReflectable> childObjectB = curField->GetValue(rttiInstanceB, &b);

						if(childObjectA != childObjectB)
						{
							if(childObjectA == nullptr || childObjectB == nullptr)
								return false;

							RTTITypeBase* childRtti = nullptr;
							if(childObjectA->GetRtti() == childObjectB->GetRtti())
								childRtti = childObjectA->GetRtti();

							if(childRtti != nullptr)
							{
								ICompare& handler = childRtti->GetCompareHandler();
								if(!handler.Run(*childObjectA, *childObjectB))
									return false;
							}
							else
								return false;
						}

						break;
					}
				case SerializableFT_Reflectable:
					{
						auto curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);

						IReflectable& childObjectA = curField->GetValue(rttiInstanceA, &a);
						IReflectable& childObjectB = curField->GetValue(rttiInstanceB, &b);

						RTTITypeBase* childRtti = nullptr;
						if(childObjectA.GetRtti() == childObjectB.GetRtti())
							childRtti = childObjectA.GetRtti();

						if(childRtti != nullptr)
						{
							ICompare& handler = childRtti->GetCompareHandler();
							if(!handler.Run(childObjectA, childObjectB))
								return false;
						}
						else
							return false;

						break;
					}
				case SerializableFT_Plain:
					{
						auto curField = static_cast<RTTIPlainFieldBase*>(curGenericField);

						u32 typeSizeA = 0;
						u32 typeSizeB = 0;
						if(curField->Schema.HasDynamicSize)
						{
							typeSizeA = curField->GetDynamicSize(rttiInstanceA, &a, false).Bytes;
							typeSizeB = curField->GetDynamicSize(rttiInstanceB, &b, false).Bytes;
						}
						else
							typeSizeA = typeSizeB = curField->Schema.Size.Bytes;

						if(typeSizeA != typeSizeB)
							return false;

						// Note: Ideally avoid doing copies here, and compare field values directly
						auto dataA = bs_managed_stack_alloc(typeSizeA);
						auto dataB = bs_managed_stack_alloc(typeSizeB);

						Bitstream streamA((uint8_t*)static_cast<void*>(dataA), typeSizeA);
						Bitstream streamB((uint8_t*)static_cast<void*>(dataB), typeSizeB);

						curField->ToStream(rttiInstanceA, &a, streamA);
						curField->ToStream(rttiInstanceB, &b, streamB);

						if(memcmp(dataA, dataB, typeSizeA) != 0)
							return false;

						break;
					}
				case SerializableFT_DataBlock:
					{
						auto curField = static_cast<RTTIManagedDataBlockFieldBase*>(curGenericField);

						u32 dataBlockSizeA = 0, dataBlockSizeB = 0;
						SPtr<DataStream> blockStreamA = curField->GetValue(rttiInstanceA, &a, dataBlockSizeA);
						SPtr<DataStream> blockStreamB = curField->GetValue(rttiInstanceB, &b, dataBlockSizeB);

						if(dataBlockSizeA != dataBlockSizeB)
							return false;

						auto dataA = bs_managed_stack_alloc(dataBlockSizeA);
						auto dataB = bs_managed_stack_alloc(dataBlockSizeB);

						blockStreamA->Read(dataA, dataBlockSizeA);
						blockStreamB->Read(dataB, dataBlockSizeB);

						if(memcmp(dataA, dataB, dataBlockSizeA) != 0)
							return false;

						break;
					}
				default:
					BS_EXCEPT(InternalErrorException, "Error encoding data. Encountered a type I don't know how to encode. Type: " + toString(u32(curGenericField->Schema.Type)) + ", Is array: " + toString(curGenericField->Schema.IsArray));
				}
			}
		}

		rtti = rtti->GetBaseClass();
	}
	while(rtti != nullptr); // Repeat until we reach the top of the inheritance hierarchy

	return true;
}
