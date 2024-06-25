//************************************ bs::framework - Copyright 2024 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Threading/BsSchedulerTicket.h"

using namespace bs;

// Note: Code ported from marl library (https://github.com/google/marl)

SchedulerTicket::SchedulerTicket(SchedulerTicketData* record)
	: mData(record)
{ }

inline SchedulerTicket::~SchedulerTicket()
{
	if(mData != nullptr)
	{
		B3DPoolDelete(mData);
		mData = nullptr;
	}
}

void SchedulerTicket::WaitUntilCalled() const
{
	Lock lock(mData->SharedData->Mutex);
	mData->IsCalledSignal.Wait(lock, [this] { return mData->IsCalled; });
}

void SchedulerTicket::TransitionToDoneState() const
{
	mData->TransitionToDoneState();
}

template <typename Function>
void SchedulerTicket::DoWhenCalled(Function&& callback) const
{
	Lock lock(mData->SharedData->Mutex);
	if(mData->IsCalled)
	{
		Scheduler::Post(std::forward<Function>(callback));
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

			Function a, b;
		};

		mData->Callback = std::move(JoinedCallbacks{ std::move(mData->Callback), std::forward<Function>(callback) });
	}
	else
		mData->Callback = std::forward<Function>(callback);
}

inline SchedulerTicketQueue::SchedulerTicketQueue(Scheduler& scheduler)
{
	mSharedData->Scheduler = &scheduler;
}

SchedulerTicket SchedulerTicketQueue::TakeTicket()
{
	SchedulerTicket output;
	TakeTickets(1, [&output](SchedulerTicket&& ticket) { output = ticket; });
	return output;
}

template <typename F>
void SchedulerTicketQueue::TakeTickets(u32 count, const F& callback)
{
	SchedulerTicketData* first = nullptr;
	SchedulerTicketData* last = nullptr;

	for(u32 ticketIndex = 0; ticketIndex < count; ++ticketIndex)
	{
		SchedulerTicketData* const record = B3DPoolNew<SchedulerTicketData>();
		record->SharedData = mSharedData;

		if (first == nullptr)
			first = record;

		if (last != nullptr)
		{
			last->Next = record;
			record->Previous = last;
		}

		last = record;

		callback(SchedulerTicket(record));
	}

	last->Next = mSharedData->LastTicket;

	Lock lock(mSharedData->Mutex);
	first->Previous = mSharedData->LastTicket->Previous;
	mSharedData->LastTicket->Previous = last;

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
