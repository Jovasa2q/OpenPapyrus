// Copyright 2017 The Abseil Authors.
// Licensed under the Apache License, Version 2.0 (the "License");
//
#include "absl/absl-internal.h"
#pragma hdrstop
#include "absl/synchronization/notification.h"

namespace absl {
ABSL_NAMESPACE_BEGIN

void Notification::Notify() 
{
	MutexLock l(&this->mutex_);
#ifndef NDEBUG
	if(ABSL_PREDICT_FALSE(notified_yet_.load(std::memory_order_relaxed))) {
		ABSL_RAW_LOG(FATAL, "Notify() method called more than once for Notification object %p", static_cast<void *>(this));
	}
#endif
	notified_yet_.store(true, std::memory_order_release);
}

Notification::~Notification() 
{
	// Make sure that the thread running Notify() exits before the object is destructed.
	MutexLock l(&this->mutex_);
}

void Notification::WaitForNotification() const 
{
	if(!HasBeenNotifiedInternal(&this->notified_yet_)) {
		this->mutex_.LockWhen(Condition(&HasBeenNotifiedInternal, &this->notified_yet_));
		this->mutex_.Unlock();
	}
}

bool Notification::WaitForNotificationWithTimeout(absl::Duration timeout) const {
	bool notified = HasBeenNotifiedInternal(&this->notified_yet_);
	if(!notified) {
		notified = this->mutex_.LockWhenWithTimeout(
			Condition(&HasBeenNotifiedInternal, &this->notified_yet_), timeout);
		this->mutex_.Unlock();
	}
	return notified;
}

bool Notification::WaitForNotificationWithDeadline(absl::Time deadline) const {
	bool notified = HasBeenNotifiedInternal(&this->notified_yet_);
	if(!notified) {
		notified = this->mutex_.LockWhenWithDeadline(
			Condition(&HasBeenNotifiedInternal, &this->notified_yet_), deadline);
		this->mutex_.Unlock();
	}
	return notified;
}

ABSL_NAMESPACE_END
}  // namespace absl
