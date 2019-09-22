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

	bool BinaryCompare::run(IReflectable& a, IReflectable& b)
	{
		mAlloc->markFrame();
		bool output = compare(a, b);
		mObjectMap.clear();
		mAlloc->clear();

		return output;
	}

	bool BinaryCompare::compare(IReflectable& a, IReflectable& b)
	{
		RTTITypeBase* rtti = a.getRTTI();

		if(rtti != b.getRTTI())
			return false;

		struct RTTIPair
		{
			RTTITypeBase* rttiA;
			RTTITypeBase* rttiB;
		};

		FrameStack<RTTIPair> rttiInstances;
		auto cleanup = impl::make_scope_guard([&]()
		{
			while (!rttiInstances.empty())
			{
				RTTIPair rttiPair = rttiInstances.top();
				rttiPair.rttiA->onSerializationEnded(&a, mContext);
				rttiPair.rttiB->onSerializationEnded(&b, mContext);
				mAlloc->destruct(rttiPair.rttiA);
				mAlloc->destruct(rttiPair.rttiB);

				rttiInstances.pop();
			}
		});

		// If an object has base classes, we need to iterate through all of them
		do
		{
			RTTITypeBase* rttiInstanceA = rtti->_clone(*mAlloc);
			RTTITypeBase* rttiInstanceB = rtti->_clone(*mAlloc);
			rttiInstances.push({ rttiInstanceA, rttiInstanceB });

			rttiInstanceA->onSerializationStarted(&a, mContext);
			rttiInstanceB->onSerializationStarted(&b, mContext);

			const UINT32 numFields = rtti->getNumFields();
			for (UINT32 i = 0; i < numFields; i++)
			{
				RTTIField* curGenericField = rtti->getField(i);
				if (curGenericField->schema.isArray)
				{
					const UINT32 arrayNumElemsA = curGenericField->getArraySize(rttiInstanceA, &a);
					const UINT32 arrayNumElemsB = curGenericField->getArraySize(rttiInstanceB, &b);

					if(arrayNumElemsA != arrayNumElemsB)
						return false;

					switch (curGenericField->schema.type)
					{
					case SerializableFT_ReflectablePtr:
					{
						auto curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);

						for (UINT32 arrIdx = 0; arrIdx < arrayNumElemsA; arrIdx++)
						{
							SPtr<IReflectable> childObjectA = curField->getArrayValue(rttiInstanceA, &a, arrIdx);
							SPtr<IReflectable> childObjectB = curField->getArrayValue(rttiInstanceB, &b, arrIdx);

							if (childObjectA != childObjectB)
							{
								if (childObjectA == nullptr || childObjectB == nullptr)
									return false;

								RTTITypeBase* childRtti = nullptr;
								if (childObjectA->getRTTI() == childObjectB->getRTTI())
									childRtti = childObjectA->getRTTI();

								if (childRtti != nullptr)
								{
									ICompare& handler = childRtti->getCompareHandler();
									if (!handler.run(*childObjectA, *childObjectB))
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
							IReflectable& childObjectA = curField->getArrayValue(rttiInstanceA, &a, arrIdx);
							IReflectable& childObjectB = curField->getArrayValue(rttiInstanceB, &b, arrIdx);

							RTTITypeBase* childRtti = nullptr;
							if (childObjectA.getRTTI() == childObjectB.getRTTI())
								childRtti = childObjectA.getRTTI();

							if (childRtti != nullptr)
							{
								ICompare& handler = childRtti->getCompareHandler();
								if (!handler.run(childObjectA, childObjectB))
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
								typeSizeA = curField->getArrayElemDynamicSize(rttiInstanceA, &a, arrIdx, false).bytes;
								typeSizeB = curField->getArrayElemDynamicSize(rttiInstanceB, &b, arrIdx, false).bytes;
							}
							else
								typeSizeA = typeSizeB = curField->schema.size.bytes;

							if(typeSizeA != typeSizeB)
								return false;

							// Note: Ideally avoid doing copies here, and compare field values directly
							auto dataA = bs_managed_stack_alloc(typeSizeA);
							auto dataB = bs_managed_stack_alloc(typeSizeB);

							Bitstream streamA((uint8_t*)static_cast<void*>(dataA), typeSizeA);
							Bitstream streamB((uint8_t*)static_cast<void*>(dataB), typeSizeB);

							curField->arrayElemToStream(rttiInstanceA, &a, arrIdx, streamA);
							curField->arrayElemToStream(rttiInstanceB, &b, arrIdx, streamB);

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

						SPtr<IReflectable> childObjectA = curField->getValue(rttiInstanceA, &a);
						SPtr<IReflectable> childObjectB = curField->getValue(rttiInstanceB, &b);

						if (childObjectA != childObjectB)
						{
							if(childObjectA == nullptr || childObjectB == nullptr)
								return false;

							RTTITypeBase* childRtti = nullptr;
							if (childObjectA->getRTTI() == childObjectB->getRTTI())
								childRtti = childObjectA->getRTTI();

							if (childRtti != nullptr)
							{
								ICompare& handler = childRtti->getCompareHandler();
								if (!handler.run(*childObjectA, *childObjectB))
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

						IReflectable& childObjectA = curField->getValue(rttiInstanceA, &a);
						IReflectable& childObjectB = curField->getValue(rttiInstanceB, &b);

						RTTITypeBase* childRtti = nullptr;
						if (childObjectA.getRTTI() == childObjectB.getRTTI())
							childRtti = childObjectA.getRTTI();

						if (childRtti != nullptr)
						{
							ICompare& handler = childRtti->getCompareHandler();
							if(!handler.run(childObjectA, childObjectB))
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
							typeSizeA = curField->getDynamicSize(rttiInstanceA, &a, false).bytes;
							typeSizeB = curField->getDynamicSize(rttiInstanceB, &b, false).bytes;
						}
						else
							typeSizeA = typeSizeB = curField->schema.size.bytes;

						if (typeSizeA != typeSizeB)
							return false;

						// Note: Ideally avoid doing copies here, and compare field values directly
						auto dataA = bs_managed_stack_alloc(typeSizeA);
						auto dataB = bs_managed_stack_alloc(typeSizeB);

						Bitstream streamA((uint8_t*)static_cast<void*>(dataA), typeSizeA);
						Bitstream streamB((uint8_t*)static_cast<void*>(dataB), typeSizeB);

						curField->toStream(rttiInstanceA, &a, streamA);
						curField->toStream(rttiInstanceB, &b, streamB);

						if (memcmp(dataA, dataB, typeSizeA) != 0)
							return false;

						break;
					}
					case SerializableFT_DataBlock:
					{
						auto curField = static_cast<RTTIManagedDataBlockFieldBase*>(curGenericField);

						UINT32 dataBlockSizeA = 0, dataBlockSizeB = 0;
						SPtr<DataStream> blockStreamA = curField->getValue(rttiInstanceA, &a, dataBlockSizeA);
						SPtr<DataStream> blockStreamB = curField->getValue(rttiInstanceB, &b, dataBlockSizeB);

						if(dataBlockSizeA != dataBlockSizeB)
							return false;

						auto dataA = bs_managed_stack_alloc(dataBlockSizeA);
						auto dataB = bs_managed_stack_alloc(dataBlockSizeB);

						blockStreamA->read(dataA, dataBlockSizeA);
						blockStreamB->read(dataB, dataBlockSizeB);

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

			rtti = rtti->getBaseClass();

		} while (rtti != nullptr); // Repeat until we reach the top of the inheritance hierarchy

		return true;
	}
}
