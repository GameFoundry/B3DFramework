//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "B3DScheduler.h"
#include "B3DSignal.h"
#include "Allocators/B3DPoolAlloc.h"

namespace b3d
{
	struct SchedulerTicketQueueData;
	struct SchedulerTicketData;

	/**
	 * Tickets allow you to schedule sequential execution of tasks. Tickets are retrieved from the SchedulerTicketQueue, and can be in three states:
	 * Waiting, Called and Done.
	 *
	 * First ticket taken from the queue will be in Called state. Any subsequent tickets will be in Waiting state. Once we call Done() on the ticket,
	 * the ticket is sent to Done state, and next ticket in line is moved from Waiting to Called state.
	 *
	 * Each ticket can have an associated callback, which will trigger when a ticket is Called.
	 */
	class B3D_EXPORT SchedulerTicket
	{
	public:
		SchedulerTicket() = default;
		SchedulerTicket(const SchedulerTicket& other) = default;
		SchedulerTicket(SchedulerTicket&& other) = default;
		SchedulerTicket& operator=(const SchedulerTicket& other) = default;
		SchedulerTicket& operator=(SchedulerTicket&& other) = default;

		/** Blocks calling code until the ticket is out of the Waiting state. */
		void WaitUntilCalled() const;

		/** Moves the ticket into Done state. Should only be called on tickets that have been Called. */
		void TransitionToDoneState() const;

		/**
		 * Registers a callback that will trigger when the ticket triggers. Multiple callbacks can be registered,
		 * in which case they will trigger in the order they were registered.
		 */
		void DoWhenCalled(Function<void()>&& callback) const;

	private:
		friend class SchedulerTicketQueue;

		SchedulerTicket(TShared<SchedulerTicketData>&& record);

		TShared<SchedulerTicketData> mData;
	};

	/** Linked list entry storing information about one ticket, and links a ticket to its previous/next ticket. */
	struct B3D_EXPORT SchedulerTicketData
	{
		inline ~SchedulerTicketData();

		/**
		 * Transitions the associated ticket into Done state. If all previous tickets are also in Done state,
		 * moves the next ticket (if any) into Called state.
		 */
		void TransitionToDoneState();

		/**
		 * Transitions the ticket into Called state, and triggers the associated callback (if any).
		 *
		 * @param	lock		Locked acquired from the ticket queue's shared mutex. Lock will be unlocked internally.
		 */
		void TransitionToCalledState(Lock& lock);

		/** Unlinks the record from the linked list. This call must be guarded by a lock from the ticket queue's mutex. */
		void RemoveFromLinkedList();

		TShared<SchedulerTicketQueueData> SharedData;
		SchedulerTicketData* Next = nullptr;
		SchedulerTicketData* Previous = nullptr;
		Function<void()> Callback;
		bool IsCalled = false;
		std::atomic<bool> IsDone = { false };
		Signal IsCalledSignal;
	};

	/** Data shared between all tickets for a particular queue. */
	struct SchedulerTicketQueueData
	{
		Scheduler* Scheduler = nullptr;
		Mutex Mutex;
		SchedulerTicketData LastTicketData;
	};

	/** Queue that hands out scheduler tickets. */
	class B3D_EXPORT SchedulerTicketQueue
	{
	public:
		SchedulerTicketQueue(Scheduler& scheduler);

		/** Returns a single ticket from the queue. */
		SchedulerTicket TakeTicket();

		/**
		 * Retrieves a number of tickets from the queue. For each ticket @p callback() is called.
		 * Callback signature must be void(SchedulerTicket&&).
		 *
		 * @param	count		Number of tickets to retrieve.
		 * @param	callback	Callback function to call for each ticket.
		 */
		template <typename Function>
		void TakeTickets(u32 count, const Function& callback);

	private:
		TShared<SchedulerTicketQueueData> mSharedData = B3DMakeShared<SchedulerTicketQueueData>();
	};

	B3D_IMPLEMENT_GLOBAL_POOL(SchedulerTicketData, 512)
}  // namespace b3d
