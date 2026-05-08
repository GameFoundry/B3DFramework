//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Threading/B3DSchedulerTicket.h"

using namespace b3d;

// Note: Code ported from marl library (https://github.com/google/marl)

SchedulerTicket::SchedulerTicket(TShared<SchedulerTicketData>&& record)
	: mData(std::move(record))
{ }

void SchedulerTicket::WaitUntilCalled() const
{
	Lock lock(mData->SharedData->Mutex);
	mData->IsCalledSignal.Wait(lock, [this] { return mData->IsCalled; });
}

void SchedulerTicket::TransitionToDoneState() const
{
	mData->TransitionToDoneState();
}

void SchedulerTicket::DoWhenCalled(Function<void()>&& callback) const
{
	Lock lock(mData->SharedData->Mutex);
	if(mData->IsCalled)
	{
		mData->SharedData->Scheduler->Post(SchedulerTask(std::move(callback), "SchedulerTicket callback"));
		return;
	}

	if(mData->Callback)
	{
		struct JoinedCallbacks
		{
			void operator()() const
			{
				a();
				b();
			}

			Function<void()> a, b;
		};

		mData->Callback = JoinedCallbacks{ std::move(mData->Callback), std::move(callback) };
	}
	else
		mData->Callback = std::move(callback);
}

inline SchedulerTicketQueue::SchedulerTicketQueue(Scheduler& scheduler)
{
	mSharedData->Scheduler = &scheduler;
}

SchedulerTicket SchedulerTicketQueue::TakeTicket()
{
	SchedulerTicket outputTicket;
	TakeTickets(1, [&outputTicket](SchedulerTicket&& ticket) { outputTicket = std::move(ticket); });
	return outputTicket;
}

template <typename F>
void SchedulerTicketQueue::TakeTickets(u32 count, const F& callback)
{
	SchedulerTicketData* first = nullptr;
	SchedulerTicketData* last = nullptr;

	for(u32 ticketIndex = 0; ticketIndex < count; ++ticketIndex)
	{
		TShared<SchedulerTicketData> record = B3DMakeSharedFromExisting(B3DPoolNew<SchedulerTicketData>(), [](SchedulerTicketData* object)
		{
			B3DPoolDelete(object);
		});
		record->SharedData = mSharedData;

		if (first == nullptr)
			first = record.get();

		if (last != nullptr)
		{
			last->Next = record.get();
			record->Previous = last;
		}

		last = record.get();

		callback(SchedulerTicket(std::move(record)));
	}

	last->Next = &mSharedData->LastTicketData;

	Lock lock(mSharedData->Mutex);
	first->Previous = mSharedData->LastTicketData.Previous;
	mSharedData->LastTicketData.Previous = last;

	if(first->Previous == nullptr)
		first->TransitionToCalledState(lock);
	else
		first->Previous->Next = first;
}

SchedulerTicketData::~SchedulerTicketData()
{
	if(SharedData != nullptr)
		TransitionToDoneState();
}

void SchedulerTicketData::TransitionToDoneState()
{
	if(IsDone.exchange(true))
		return;

	Lock lock(SharedData->Mutex);
	auto recordToCallNext = (Previous == nullptr && Next != nullptr) ? Next : nullptr;

	RemoveFromLinkedList();

	if(recordToCallNext != nullptr)
		recordToCallNext->TransitionToCalledState(lock);
}

void SchedulerTicketData::TransitionToCalledState(Lock& lock)
{
	if(IsCalled)
		return;

	IsCalled = true;
	Function<void()> callback;
	std::swap(callback, Callback);
	IsCalledSignal.NotifyAll();
	lock.unlock();

	if(callback)
		SharedData->Scheduler->Post(std::move(callback));
}

void SchedulerTicketData::RemoveFromLinkedList()
{
	if(Previous != nullptr)
		Previous->Next = Next;

	if(Next != nullptr)
		Next->Previous = Previous;

	Previous = nullptr;
	Next = nullptr;
}
