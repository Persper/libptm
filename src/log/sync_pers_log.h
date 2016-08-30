//
// The Persper License
//
// Copyright (c) 2016 Persper Technologies Co., Ltd.
//

#ifndef PERSPER_LIBPM_SYNC_PERS_LOG_H_
#define PERSPER_LIBPM_SYNC_PERS_LOG_H_

#include <cassert>
#include <stdexcept>
#include "primitive_log.h"

namespace persper {

struct PersCommit {
  virtual int operator()(void *data, size_t size) = 0;
};

/**
 * A synchronous persistent log.
*/
template <class PersCommit>
class SyncPersLog {
 public:
  /**
   * Constructs the log on a (local) DRAM cache for the (remote) persistent
   * storage. 
  */
  SyncPersLog(void *cache, size_t size) throw (std::exception);

  /**
   * Appends an amount of data to the end of the log cache.
   * This operation does not persist data.
   *
   * @param meta Metadata about the data to append. Typically it includes the
   * original/home address of the logged data. Null if it should be skipped.
   * @param data Address of the data to append. Null if no data payload.
   * @param size Size of the data to append.
   * @return Zero on success. ENOSPC if there is no enough space. 
  */
  template <typename MetaData>
  int Append(MetaData *meta, void *data, size_t size);

  /**
   * Gets the occupied size of the cache.
   */
  size_t Occupation();

  /**
   * Persists all previously appended data in the log or to the remote node.
   * Once this operation is performed, the log cache is cleared.
   *
   * @return Zero on sucess. EINVAL if the log state is not valid for commit.
   * Otherwise, according to underlying pcommit().
  */
  int Commit();

  /**
   * Gets the head entry of the log.
   * @see LogEntry
  */
  LogEntry *head() const { return log_.head(); }

  /**
   * Gets the end entry of the log. This entry is invaid.
   * @see LogEntry
   */
  LogEntry *end() const { return log_.end(); }

 private:
  int DoAppend(void *data, size_t size);

  PrimitiveLog log_;
  PersCommit pcommit_;
};

// Implementation of SyncPersLog

template <class PersCommit>
SyncPersLog<PersCommit>::SyncPersLog(void *cache, size_t size)
    throw (std::exception) {
  if (log_.Init(cache, size)) {
    throw std::invalid_argument("Invalid format of the log cache!"
        " (Not zeroed?)");
  }
}

template <class PersCommit>
int SyncPersLog<PersCommit>::DoAppend(void *data, size_t size) {
  if (!data) return 0;
  void *dest = nullptr;
  if (log_.empty()) {
    LogEntry *entry = log_.Append(size);
    if (!entry) return ENOSPC;
    dest = entry->data();
  } else {
    int err = log_.Extend(log_.head(), size);
    assert(!err || err == ENOSPC);
    if (err) return err;
    dest = (char *)log_.head()->data() + log_.head()->size - size;
  }
  memcpy(dest, data, size);
  return 0;
}

template <class PersCommit>
template <typename MetaData>
int SyncPersLog<PersCommit>::Append(MetaData *meta, void *data, size_t size) {
  if (log_.empty()) {
    pcommit_(log_.head(), sizeof(LogEntry));
  }
  if (DoAppend(meta, sizeof(MetaData))) {
    return ENOSPC;
  }
  if (DoAppend(data, size)) {
    return ENOSPC;
  }
  return 0;
}

template <class PersCommit>
size_t SyncPersLog<PersCommit>::Occupation() {
  return (char *)log_.end() + sizeof(LogEntry) - (char *)log_.data();
}

template <class PersCommit>
int SyncPersLog<PersCommit>::Commit() {
  LogEntry *entry = log_.head();
  if (!entry->size) return EINVAL;
  // First persistent commit
  int err = pcommit_(entry->data(), entry->size + sizeof(LogEntry));
  if (err) return err;
  // Second persistent commit
  err = pcommit_(entry, sizeof(LogEntry));
  if (err) return err;
  log_.Truncate();
  entry = log_.Rewind();
  assert(entry);
  return 0;
}

} // namespace persper

#endif // PERSPER_LIBPM_SYNC_PERS_LOG_H_
