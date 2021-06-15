#include <stdio.h>

#include <dbus/dbus.h>

int main (int argc, char *argv[]) 
{
  printf ("Hello World!\n");

  DBusError err;
  DBusConnection* conn;
  // initialise the errors
  dbus_error_init (&err);

  //check that dbus is working
  conn = dbus_bus_get_private (DBUS_BUS_SYSTEM, &err);
  if (dbus_error_is_set (&err)) 
  { 
    fprintf (stderr, "Connection Error (%s)\n", err.message); 
    dbus_error_free (&err); 
  }
  if (NULL == conn) { 
    return 1; 
  }
  dbus_connection_close (conn);

  return 0;
}
