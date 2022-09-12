//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/BsBinaryCompare.h"
#include "Reflection/BsRTTIType.h"
#include "FileSystem/BsDataStream.h"

namespace bs
{
	namespace impl
	{
		template<class T>
		struct ScopeGuard
		{
			~ScopeGuard()
			{
				callback();
			}

			T callback;
		};

		template<class T>
		ScopeGuard<T> make_scope_guard(T callback) { return ScopeGuard<T>{callback}; }
	}

	BinaryCompare::BinaryCompare()
		:mAlloc(&gFrameAlloc())
	{ }

	bool BinaryCompare::Run(IReflectable& a, IReflectable& b)
	{
		mAlloc->MarkFrame();
		bool output = compare(a, b);
		mObjectMap.Clear();
		mAlloc->Clear();

		return output;
	}

	bool BinaryCompare::Compare(IReflectable& a, IReflectable& b)
	{
		RTTITypeBase* rtti = a.GetRTTI();

		if(rtti != b.GetRTTI())
			return false;

		struct RTTIPair
		{
			RTTITypeBase* rttiA;
			RTTITypeBase* rttiB;
		};

		FrameStack<RTTIPair> rttiInstances;
		auto cleanup = impl::make_scope_guard([&]()
		{
			while (!rttiInstances.Empty())
			{
				RTTIPair rttiPair = rttiInstances.Top();
				rttiPair.rttiA->OnSerializationEnded(&a, mContext);
				rttiPair.rttiB->OnSerializationEnded(&b, mContext);
				mAlloc->Destruct(rttiPair.rttiA);
				mAlloc->Destruct(rttiPair.rttiB);

				rttiInstances.Pop();
			}
		});

		// If an object has base classes, we need to iterate through all of them
		do
		{
			RTTITypeBase* rttiInstanceA = rtti->_clone(*mAlloc);
			RTTITypeBase* rttiInstanceB = rtti->_clone(*mAlloc);
			rttiInstances.Push({ rttiInstanceA, rttiInstanceB });

			rttiInstanceA->OnSerializationStarted(&a, mContext);
			rttiInstanceB->OnSerializationStarted(&b, mContext);

			const UINT32 numFields = rtti->GetNumFields();
			for (UINT32 i = 0; i < numFields; i++)
			{
				RTTIField* curGenericField = rtti->GetField(i);
				if (curGenericField->schema.isArray)
				{
					const UINT32 arrayNumElemsA = curGenericField->GetArraySize(rttiInstanceA, &a);
					const UINT32 arrayNumElemsB = curGenericField->GetArraySize(rttiInstanceB, &b);

					if(arrayNumElemsA != arrayNumElemsB)
						return false;

					switch (curGenericField->schema.type)
					{
					case SerializableFT_ReflectablePtr:
					{
						auto curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);

						for (UINT32 arrIdx = 0; arrIdx < arrayNumElemsA; arrIdx++)
						{
							SPtr<IReflectable> childObjectA = curField->GetArrayValue(rttiInstanceA, &a, arrIdx);
							SPtr<IReflectable> childObjectB = curField->GetArrayValue(rttiInstanceB, &b, arrIdx);

							if (childObjectA != childObjectB)
							{
								if (childObjectA == nullptr || childObjectB == nullptr)
									return false;

								RTTITypeBase* childRtti = nullptr;
								if (childObjectA->GetRTTI() == childObjectB->getRTTI())
									childRtti = childObjectA->GetRTTI();

								if (childRtti != nullptr)
								{
									ICompare& handler = childRtti->GetCompareHandler();
									if (!handler.Run(*childObjectA, *childObjectB))
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

						for (UINT32 arrIdx = 0; arrIdx < arrayNumElemsA; arrIdx++)
						{
							IReflectable& childObjectA = curField->GetArrayValue(rttiInstanceA, &a, arrIdx);
							IReflectable& childObjectB = curField->GetArrayValue(rttiInstanceB, &b, arrIdx);

							RTTITypeBase* childRtti = nullptr;
							if (childObjectA.GetRTTI() == childObjectB.getRTTI())
								childRtti = childObjectA.GetRTTI();

							if (childRtti != nullptr)
							{
								ICompare& handler = childRtti->GetCompareHandler();
								if (!handler.Run(childObjectA, childObjectB))
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

						for (UINT32 arrIdx = 0; arrIdx < arrayNumElemsA; arrIdx++)
						{
							UINT32 typeSizeA = 0;
							UINT32 typeSizeB = 0;
							if (curField->schema.hasDynamicSize)
							{
								typeSizeA = curField->GetArrayElemDynamicSize(rttiInstanceA, &a, arrIdx, false).bytes;
								typeSizeB = curField->GetArrayElemDynamicSize(rttiInstanceB, &b, arrIdx, false).bytes;
							}
							else
								typeSizeA = typeSizeB = curField->schema.size.bytes;

							if(typeSizeA != typeSizeB)
								return false;

							// Note: Ideally avoid doing copies here, and compare field values directly
							auto dataA = bs_managed_stack_alloc(typeSizeA);
							auto dataB = bs_managed_stack_alloc(typeSizeB);

							Bitstream StreamA((uint8_t*)static_cast<void*>(dataA), typeSizeA);
							Bitstream StreamB((uint8_t*)static_cast<void*>(dataB), typeSizeB);

							curField->ArrayElemToStream(rttiInstanceA, &a, arrIdx, streamA);
							curField->ArrayElemToStream(rttiInstanceB, &b, arrIdx, streamB);

							if(memcmp(dataA, dataB, typeSizeA) != 0)
								return false;
						}

						break;
					}
					default:
						BS_EXCEPT(InternalErrorException,
							"Error encoding data. Encountered a type I don't know how to encode. Type: " + toString(UINT32(curGenericField->schema.type)) +
							", Is array: " + toString(curGenericField->schema.isArray));
					}
				}
				else
				{
					switch (curGenericField->schema.type)
					{
					case SerializableFT_ReflectablePtr:
					{
						auto curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);

						SPtr<IReflectable> childObjectA = curField->GetValue(rttiInstanceA, &a);
						SPtr<IReflectable> childObjectB = curField->GetValue(rttiInstanceB, &b);

						if (childObjectA != childObjectB)
						{
							if(childObjectA == nullptr || childObjectB == nullptr)
								return false;

							RTTITypeBase* childRtti = nullptr;
							if (childObjectA->GetRTTI() == childObjectB->getRTTI())
								childRtti = childObjectA->GetRTTI();

							if (childRtti != nullptr)
							{
								ICompare& handler = childRtti->GetCompareHandler();
								if (!handler.Run(*childObjectA, *childObjectB))
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
						if (childObjectA.GetRTTI() == childObjectB.getRTTI())
							childRtti = childObjectA.GetRTTI();

						if (childRtti != nullptr)
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

						UINT32 typeSizeA = 0;
						UINT32 typeSizeB = 0;
						if (curField->schema.hasDynamicSize)
						{
							typeSizeA = curField->GetDynamicSize(rttiInstanceA, &a, false).bytes;
							typeSizeB = curField->GetDynamicSize(rttiInstanceB, &b, false).bytes;
						}
						else
							typeSizeA = typeSizeB = curField->schema.size.bytes;

						if (typeSizeA != typeSizeB)
							return false;

						// Note: Ideally avoid doing copies here, and compare field values directly
						auto dataA = bs_managed_stack_alloc(typeSizeA);
						auto dataB = bs_managed_stack_alloc(typeSizeB);

						Bitstream StreamA((uint8_t*)static_cast<void*>(dataA), typeSizeA);
						Bitstream StreamB((uint8_t*)static_cast<void*>(dataB), typeSizeB);

						curField->ToStream(rttiInstanceA, &a, streamA);
						curField->ToStream(rttiInstanceB, &b, streamB);

						if (memcmp(dataA, dataB, typeSizeA) != 0)
							return false;

						break;
					}
					case SerializableFT_DataBlock:
					{
						auto curField = static_cast<RTTIManagedDataBlockFieldBase*>(curGenericField);

						UINT32 dataBlockSizeA = 0, dataBlockSizeB = 0;
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
						BS_EXCEPT(InternalErrorException,
							"Error encoding data. Encountered a type I don't know how to encode. Type: " + toString(UINT32(curGenericField->schema.type)) +
							", Is array: " + toString(curGenericField->schema.isArray));
					}
				}
			}

			rtti = rtti->GetBaseClass();

		} while (rtti != nullptr); // Repeat until we reach the top of the inheritance hierarchy

		return true;
	}
}
