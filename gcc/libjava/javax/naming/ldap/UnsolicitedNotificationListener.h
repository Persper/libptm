
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_naming_ldap_UnsolicitedNotificationListener__
#define __javax_naming_ldap_UnsolicitedNotificationListener__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace javax
  {
    namespace naming
    {
      namespace event
      {
          class NamingExceptionEvent;
      }
      namespace ldap
      {
          class UnsolicitedNotificationEvent;
          class UnsolicitedNotificationListener;
      }
    }
  }
}

class javax::naming::ldap::UnsolicitedNotificationListener : public ::java::lang::Object
{

public:
  virtual void notificationReceived(::javax::naming::ldap::UnsolicitedNotificationEvent *) = 0;
  virtual void namingExceptionThrown(::javax::naming::event::NamingExceptionEvent *) = 0;
  static ::java::lang::Class class$;
} __attribute__ ((java_interface));

#endif // __javax_naming_ldap_UnsolicitedNotificationListener__