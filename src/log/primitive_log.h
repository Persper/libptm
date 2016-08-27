//
// The Persper License
//
// Copyright (c) 2016 Persper Technologies Co., Ltd.
//

#ifndef PERSPER_LIBPM_PRIMITIVE_LOG_H_
#define PERSPER_LIBPM_PRIMITIVE_LOG_H_

namespace persper {

/**
 * Header of a log entry to record the size of the logged data (payload).
*/
struct LogEntry {
  /**
   * Size of the payload.
   * Zero denotes end of a primitive log. A negative size indicates the entry
   * is invalid, but its absolute value can be used to retrieve the next entry.
  */
  int size;

  /**
   * Get the address of the payload in the log.
  */
  void *data() const { return (void *)((char *)this + sizeof(size)); }
};


/**
 * A primitive log that involves no memory allocation.
 * The log offers no public constructor. It should be put on a zeroed memory
 * area and starts with a Init() call. The log is customizable to locate in
 * persistent memory, in the following way: any returned log entry (except for
 * one returned from a const function) is made persistent by the caller in
 * addition to the payload, and it is safe to call Init() to restore the log on
 * recovery.
*/
class PrimitiveLog {
 public:
  /**
   * Initializes the log of a certain size from where the log is located.
   *
   * @return Zero on success. Otherwise, inconsistency is encountered.
  */
  int Init(size_t size);

  /**
   * Appends an amount of data to the end of the log.
   * This function only prepares the space for the append but does not actually
   * allocate or copy data. If the log is persistent, the caller has to persist
   * a length of (size + 2 * sizeof(LogEntry)) memory space starting from the
   * returned address after calling this function.
   *
   * @return The appended log entry, by which the size and address of the
   * payload can be retrieved. Null if there is no enough space.
   * @see LogEntry
  */
  LogEntry *Append(int size);

  /**
   * Extends an existing entry at the end of the log.
   * If the log is persistent, the caller has to persist a length of (size + 2
   * * sizeof(LogEntry)) memory space starting from the input entry address
   * after calling this function.

   * @param addition The number of bytes added to the entry.
   * @return The size of the extended entry. Negative if there is no enough
   * space in the log.
  */
  int Extend(LogEntry *entry, int addition);

  /**
   * Gets the head entry of the log.
   *
   * @return A log entry header, by which the size and address of the payload
   * can be retrieved.
   * @see LogEntry
  */
  LogEntry *Head() const { return head_entry_; }

  /**
   * Truncates the head entry of the log.
   *
   * @return The removed log entry. This entry has to be persisted as well if
   * the log is persistent. Null if the log is empty.
   * @see LogEntry
  */
  LogEntry *Truncate();

  /**
   * Empties and resets the log.
   *
   * @return The head/end entry of the log. It has to be persisted if the log
   * is in persistent memory.
  */
  LogEntry *Rewind();

 private:
  PrimitiveLog();
  LogEntry *NextEntry(LogEntry *cur, size_t size);

  size_t size_;
  LogEntry *head_entry_;
  LogEntry *end_entry_;
};

// Implementation of PrimitiveLog

LogEntry *PrimitiveLog::NextEntry(LogEntry *cur, size_t size) {
  return (LogEntry *)((char *)cur->data() + size);
}

int PrimitiveLog::Init(size_t size) {
  size_ = size;
  head_entry_ = (LogEntry *)((char *)this + sizeof(PrimitiveLog));
  while (head_entry_->size < 0) {
    head_entry_ = NextEntry(head_entry_, - head_entry_->size);
    if ((char *)head_entry_ - (char *)this >= size_) {
      return EFAULT;
    }
  }
  end_entry_ = head_entry_;
  while (end_entry_->size) {
    end_entry_ = NextEntry(end_entry_, end_entry_->size);
    if ((char *)end_entry_ - (char *)this >= size_) {
      return EFAULT;
    }
  }
  return 0;
}

LogEntry *PrimitiveLog::Append(int size) {
  LogEntry *entry = end_entry_;
  end_entry_ = NextEntry(end_entry_, size);
  if ((char *)end_entry_ - (char *)this > size_ - sizeof(LogEntry)) {
    end_entry_ = entry;
    return nullptr;
  } else {
    entry->size = size;
    end_entry_->size = 0;
    return entry;
  }
}

int PrimitiveLog::Extend(LogEntry *last_entry, int addition) {
  if (NextEntry(last_entry, last_entry->size) != end_entry_) {
    return EINVAL;
  }
  LogEntry *entry = end_entry_;
  end_entry_ = (LogEntry *)((char *)end_entry_ + addition);
  if ((char *)end_entry_ - (char *)this > size_ - sizeof(LogEntry)) {
    end_entry_ = entry;
    return -ENOSPC;
  } else {
    last_entry->size += addition;
    end_entry_->size = 0;
  }
  return last_entry->size;
}

LogEntry *PrimitiveLog::Truncate() {
  if (head_entry_ == end_entry_) return nullptr;
  LogEntry *entry = head_entry_;
  head_entry_ = NextEntry(head_entry_, head_entry_->size);
  entry->size = - entry->size;
  return entry;
}

LogEntry *PrimitiveLog::Rewind() {
  head_entry_ = end_entry_ = (LogEntry *)((char *)this + sizeof(PrimitiveLog));
  end_entry_->size = 0;
  return end_entry_;
}

} // namespace persper

#endif // PERSPER_LIBPM_PRIMITIVE_LOG_H_
