/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/
#include "stddef.h"
#include "string.h"
#include <stdlib.h>
#include <stdio.h>

#include "service.h"
#include "server.h"
#include "dbusutils.h"

server_t *server_new(const char *object_path, service_t *services)
{

  if ( NULL == object_path )
  {
    return NULL;
  }

  server_t *server = calloc(1, sizeof(*server));
  if( NULL == server)
  {
    return NULL;
  }

  server->object_path = strdup(object_path);  
  server->services = services;

  return server;
}

void server_free(server_t *server)
{
  if (server == NULL)
  {
    return;
  }

  free(server->object_path);
  server->object_path = NULL;

  service_t *tmp = NULL;
  while (server->services)
  {
    tmp = server->services->next;
    service_free(server->services);
    server->services = tmp;
  }
  
  free(server);
}

service_t* server_get_service(server_t *server, const char *service_uuid)
{
  service_t *service = server->services;
  while(service)
  {
    if(strcmp(service_uuid, service->UUID) == 0)
    {
      return service;
    }
    service = service->next;
  }

  return NULL; 
}

bool server_add_service(server_t *server, service_t *service)
{
  if (server_get_service(server, service->UUID))
  {
    return false;
  }

  service->next = server->services;
  server->services = service;
  return true;
}


void server_get_managed_objects(DBusConnection *connection, DBusMessage *message, server_t *server)
{

}
