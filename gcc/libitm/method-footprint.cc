/* Copyright (C) 2008-2015 Free Software Foundation, Inc.
   Copyright (C) 2016 Persper Inc.

   Contributed by Richard Henderson <rth@redhat.com>.
   Contributed by Jinglei Ren <jinglei@persper.com>.

   This file is part of the GNU Transactional Memory Library (libitm).

   Libitm is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   Libitm is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
   more details.

   Under Section 7 of GPL version 3, you are granted additional
   permissions described in the GCC Runtime Library Exception, version
   3.1, as published by the Free Software Foundation.

   You should have received a copy of the GNU General Public License and
   a copy of the GCC Runtime Library Exception along with this program;
   see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include "libitm_i.h"
#include <stdio.h>

using namespace GTM;

namespace {

struct serial_mg : public method_group
{
  virtual void init() { }
  virtual void fini() { }
};

static serial_mg o_serial_mg;

static size_t written_bytes(0);

class footprint_dispatch : public abi_dispatch
{
 public:
  footprint_dispatch() : abi_dispatch(false, true, true, false,
      gtm_thread::STATE_SERIAL | gtm_thread::STATE_IRREVOCABLE, &o_serial_mg)
  { }

 protected:
  footprint_dispatch(bool ro, bool wt, bool uninstrumented,
      bool closed_nesting, uint32_t requires_serial, method_group* mg) :
    abi_dispatch(ro, wt, uninstrumented, closed_nesting, requires_serial, mg)
  { }

  // Transactional loads and stores simply access memory directly.
  // These methods are static to avoid indirect calls, and will be used by the
  // virtual ABI dispatch methods or by static direct-access methods created
  // below.
  template <typename V> static V load(const V* addr, ls_modifier mod)
  {
    return *addr;
  }
  template <typename V> static void store(V* addr, const V value,
      ls_modifier mod)
  {
    written_bytes += sizeof(V);
    *addr = value;
  }

 public:
  static void memtransfer_static(void *dst, const void* src, size_t size,
      bool may_overlap, ls_modifier dst_mod, ls_modifier src_mod)
  {
    written_bytes += size;
    if (!may_overlap)
      ::memcpy(dst, src, size);
    else
      ::memmove(dst, src, size);
  }

  static void memset_static(void *dst, int c, size_t size, ls_modifier mod)
  {
    written_bytes += size;
    ::memset(dst, c, size);
  }

  CREATE_DISPATCH_METHODS(virtual, )
  CREATE_DISPATCH_METHODS_MEM()

  virtual gtm_restart_reason begin_or_restart() { return NO_RESTART; }

  virtual bool trycommit(gtm_word& priv_time)
  {
    fprintf(stdout, "%ld\n", written_bytes);
    return true;
  }

  virtual void rollback(gtm_transaction_cp *cp) { abort(); }
};

} // anon namespace

static const footprint_dispatch o_footprint_dispatch;

abi_dispatch *
GTM::dispatch_footprint ()
{
  return const_cast<footprint_dispatch *>(&o_footprint_dispatch);
}

