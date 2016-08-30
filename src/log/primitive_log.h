//
// The Persper License
//
// Copyright (c) 2016 Persper Technologies Co., Ltd.
//

#ifndef PERSPER_LIBPM_PRIMITIVE_LOG_H_
#define PERSPER_LIBPM_PRIMITIVE_LOG_H_

#include <cstdlib>

namespace persper {

/**
 * Header of a log entry to record the size of the logged data (payload).
*/
struct LogEntry {
  /**
   * Size of the payload.
   * Zero denotes end of a primitive log. A negative size indicates the entry
   * is invalid, but its absolute value can be used to retrieve the next entry
   * on recovery.
  */
  int size;

  /**
   * Gets the address of the payload in the log.
  */
  void *data() const { return (void *)((char *)this + sizeof(size)); }

  /**
   * Gets the next entry if it exists.
   *
   * @param skip Length to skip to locate the next entry. If not specified, it
   * is the size of this entry.
   */
  LogEntry *next(size_t skip = 0) const {
    return (LogEntry *)((char *)data() + (skip ? skip : labs(size)));
  }
};


/**
 * A primitive log that involves no memory allocation and enables support for
 * atomic persistence by its interface design.
 *
 * The log should be put on either a zeroed memory area for initialization or
 * an existing log area for restoral. The log can locate in persistent memory,
 * as long as the user follows the atomic persistence rule (APR): For any
 * returned log entry (except for one returned from a const function), the user
 * needs to call pcommit() twice -- one for data of the new entry and its
 * following end entry (in total, a length of size + sizeof(LogEntry)), and the
 * other for the new entry (a length of another sizeof(LogEntry)) to enable
 * visibility of data.
*/
class PrimitiveLog {
 public:
  /**
   * Initializes or restores the log from a memory region.
   *
   * @return Zero on success. Otherwise, inconsistency is encountered.
  */
  int Init(void *data, size_t size);

  /**
   * Appends an amount of data to the end of the log.
   * This function only prepares the space for the append but does not actually
   * allocate or copy data. If the log is persistent, the caller has to persist
   * a length of (size + 2 * sizeof(LogEntry)) memory space starting from the
   * returned address following the APR.
   *
   * @return The appended log entry, by which the size and address of the
   * payload can be retrieved. Null if the input is invalid or there is no
   * enough space.
   * @see LogEntry
  */
  LogEntry *Append(int size);

  /**
   * Extends an existing entry at the end of the log.
   * If the log is persistent, the caller has to persist a length of (size + 2
   * * sizeof(LogEntry)) memory space starting from the input entry address
   * following the APR.
   *
   * @param last The last entry of the log.
   * @param addition The number of bytes added to the entry.
   * @return Zero on success. EINVAL indicates the input entry is not the last
   * one -- only the last entry can be extended. ENOSPC indicates there is no
   * enough space in the log.
  */
  int Extend(LogEntry *last, int addition);

  /**
   * Gets the head entry of the log.
   * @see LogEntry
  */
  LogEntry *head() const { return head_entry_; }

  /**
   * Gets the end entry of the log. This entry is invaid.
   * @see LogEntry
   */
  LogEntry *end() const { return end_entry_; }

  /**
   * Whether the log is empty.
  */
  bool empty() const { return head_entry_ == end_entry_; }

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
   * Note that this operation preserves the last entry if it is not truncated.
   * However, this operation is only viable if the log is empty or has one last
   * entry (in case the last entry needs more space to extend).
   *
   * @param last The last entry of the log if it exists and should be
   * preserved.  @return The new head entry of the log. It has to be persisted
   * if the log is in persistent memory. Particularly, if the last entry is
   * preserved, the caller should copy data of the last entry to the new entry
   * and persist a length of (size + 2 * sizeof(LogEntry)) data following the
   * atomic persitence rule. Finally, a null is returned if more entries than
   * the last one is valid, or by all means there is no enough space for the
   * last entry so that the caller has to split the oversized log operation.
  */
  LogEntry *Rewind(LogEntry *last = nullptr);

  void *data() const { return data_; }

 private:
  void *data_ = nullptr;
  size_t size_ = 0;
  LogEntry *head_entry_ = nullptr;
  LogEntry *end_entry_ = nullptr;
};

// Implementation of PrimitiveLog

int PrimitiveLog::Init(void *data, size_t size) {
  data_ = data;
  size_ = size;
  if (!data_ || size_ < sizeof(LogEntry)) return EFAULT;

  head_entry_ = (LogEntry *)data_;
  while (head_entry_->size < 0) {
    head_entry_ = head_entry_->next();
    if ((char *)head_entry_ - (char *)data_ >= size_) {
      return EFAULT;
    }
  }
  end_entry_ = head_entry_;
  while (end_entry_->size) {
    end_entry_ = end_entry_->next();
    if ((char *)end_entry_ - (char *)data_ >= size_) {
      return EFAULT;
    }
  }
  return 0;
}

LogEntry *PrimitiveLog::Append(int size) {
  if (size <= 0) return nullptr;
  LogEntry *entry = end_entry_;
  end_entry_ = end_entry_->next(size);
  if ((char *)end_entry_ - (char *)data_ > size_ - sizeof(LogEntry)) {
    end_entry_ = entry;
    return nullptr;
  } else {
    entry->size = size;
    end_entry_->size = 0;
    return entry;
  }
}

int PrimitiveLog::Extend(LogEntry *last, int addition) {
  if (last->next() != end_entry_) {
    return EINVAL;
  }
  LogEntry *entry = end_entry_;
  end_entry_ = (LogEntry *)((char *)end_entry_ + addition);
  if ((char *)end_entry_ - (char *)data_ > size_ - sizeof(LogEntry)) {
    end_entry_ = entry;
    return ENOSPC;
  } else {
    last->size += addition;
    end_entry_->size = 0;
  }
  return 0;
}

LogEntry *PrimitiveLog::Truncate() {
  if (empty()) return nullptr;
  LogEntry *entry = head_entry_;
  head_entry_ = head_entry_->next();
  entry->size = - entry->size;
  return entry;
}

LogEntry *PrimitiveLog::Rewind(LogEntry *last) {
  if (!last) {
    if (head_entry_ != end_entry_) return nullptr;
    head_entry_ = (LogEntry *)data_;
    end_entry_ = head_entry_;
    end_entry_->size = 0;
    return end_entry_;
  } else {
    if (last != head_entry_ || last->next() != end_entry_) {
      return nullptr;
    }
    head_entry_ = (LogEntry *)data_;
    if ((char *)head_entry_ + last->size + sizeof(LogEntry) > (char *)last) {
      head_entry_ = last;
      return nullptr;
    }
    end_entry_ = head_entry_;
    return Append(last->size);
  }
}

} // namespace persper

#endif // PERSPER_LIBPM_PRIMITIVE_LOG_H_
