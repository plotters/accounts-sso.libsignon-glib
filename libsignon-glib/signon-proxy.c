/* vi: set et sw=4 ts=4 cino=t0,(0: */
/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of libsignon-glib
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
 *
 * Contact: Alberto Mardegan <alberto.mardegan@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include "signon-proxy.h"
#include "signon-internals.h"
#include <dbus/dbus-glib-lowlevel.h>
#include <dbus/dbus-glib.h>

G_DEFINE_TYPE (SignonProxy, signon_proxy, DBUS_TYPE_G_PROXY);

static GHashTable *thread_proxies = NULL;
static GMutex thread_proxies_mutex;

static SignonProxy *
signon_proxy_get_singleton ()
{
    SignonProxy *signon_proxy = NULL;

    g_mutex_lock (&thread_proxies_mutex);

    if (thread_proxies != NULL)
    {
        signon_proxy = g_hash_table_lookup (thread_proxies,
                                            g_thread_self ());
    }

    g_mutex_unlock (&thread_proxies_mutex);
    return signon_proxy;
}

static void
signon_proxy_set_singleton (SignonProxy *signon_proxy)
{
    g_mutex_lock (&thread_proxies_mutex);

    if (thread_proxies == NULL)
    {
        thread_proxies = g_hash_table_new (g_direct_hash, g_direct_equal);
    }

    if (signon_proxy != NULL)
    {
        g_hash_table_insert (thread_proxies, g_thread_self (), signon_proxy);
    }
    else
    {
        g_hash_table_remove (thread_proxies, g_thread_self ());
    }

    g_mutex_unlock (&thread_proxies_mutex);
}

static void
signon_proxy_init (SignonProxy *self)
{
}

static void
signon_proxy_dispose (GObject *object)
{
    /* We must close the D-Bus connection, because it's a private one */
    DBusGConnection *g_connection = NULL;

    g_object_get (object, "connection", &g_connection, NULL);
    if (g_connection != NULL)
    {
        DBusConnection *connection =
            dbus_g_connection_get_connection (g_connection);
        if (connection != NULL)
            dbus_connection_close (connection);
    }

    signon_proxy_set_singleton (NULL);

    G_OBJECT_CLASS (signon_proxy_parent_class)->dispose (object);
}

static void
signon_proxy_finalize (GObject *object)
{
    G_OBJECT_CLASS (signon_proxy_parent_class)->finalize (object);
}

static void
signon_proxy_class_init (SignonProxyClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = signon_proxy_dispose;
    object_class->finalize = signon_proxy_finalize;
}

SignonProxy *
signon_proxy_new ()
{
    SignonProxy *proxy;
    GError *error = NULL;

    /* Use a singleton object per thread; sharing the DBusGProxy among
     * different threads is not advisable, see:
     * https://bugs.freedesktop.org/show_bug.cgi?id=857
     */
    proxy = signon_proxy_get_singleton ();
    if (proxy != NULL)
    {
        return g_object_ref (proxy);
    }

    DBusGConnection *connection =
        dbus_g_bus_get_private (DBUS_BUS_SESSION, NULL, &error);

    if (error)
    {
        g_warning ("%s returned error: %s", G_STRFUNC, error->message);
        g_error_free (error);
        return NULL;
    }

    proxy = g_object_new (SIGNON_TYPE_PROXY,
                          "name", SIGNOND_SERVICE,
                          "path", SIGNOND_DAEMON_OBJECTPATH,
                          "interface", SIGNOND_DAEMON_INTERFACE,
                          "connection", connection,
                          NULL);

    dbus_g_connection_unref (connection);

    signon_proxy_set_singleton (proxy);

    return proxy;
}
