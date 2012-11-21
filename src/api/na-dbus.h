/*
 * Caja-Actions
 * A Caja extension which offers configurable context menu actions.
 *
 * Copyright (C) 2005 The MATE Foundation
 * Copyright (C) 2006-2008 Frederic Ruaudel and others (see AUTHORS)
 * Copyright (C) 2009-2012 Pierre Wieser and others (see AUTHORS)
 *
 * Caja-Actions is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General  Public  License  as
 * published by the Free Software Foundation; either  version  2  of
 * the License, or (at your option) any later version.
 *
 * Caja-Actions is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even  the  implied  warranty  of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See  the  GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public  License
 * along with Caja-Actions; see the file  COPYING.  If  not,  see
 * <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *   Frederic Ruaudel <grumz@grumz.net>
 *   Rodrigo Moya <rodrigo@mate-db.org>
 *   Pierre Wieser <pwieser@trychlos.org>
 *   ... and many others (see AUTHORS)
 */

#ifndef __CAJA_ACTIONS_API_NA_DBUS_H__
#define __CAJA_ACTIONS_API_NA_DBUS_H__

/**
 * SECTION: dbus
 * @title: D-Bus
 * @short_description: The D-Bus Services
 * @include: caja-actions/na-dbus.h
 *
 * &prodname;, through its <emphasis>tracker</emphasis> plugin, exposes
 * several D-Bus interfaces. These interfaces may be queried in order to get
 * informations about current selection, &prodname; status, and so on.
 *
 * <note>
 *   <para>
 *    To be really clear, &prodname; relies on &caja; in order
 *    to be able to register its own D-Bus services via the
 *    <emphasis>tracker</emphasis> plugin.
 *   </para>
 *   <para>
 *    If &caja; does not run, or the <emphasis>tracker</emphasis>
 *    plugin is not loaded at &caja; startup, then these D-Bus
 *    services will not be available.
 *   </para>
 * </note>
 */

#include <glib.h>

G_BEGIN_DECLS

/**
 * CAJA_ACTIONS_DBUS_SERVICE:
 *
 * This is the &laquo;&nbsp;well-known&nbsp;&raquo; name that
 * &prodname; reserves on D-Bus session bus.
 */
#define CAJA_ACTIONS_DBUS_SERVICE            "org.caja-actions.DBus"

/**
 * CAJA_ACTIONS_DBUS_TRACKER_PATH:
 *
 * The D-Bus path of the <emphasis>tracker</emphasis> object.
 *
 * When requested through the <methodname>Introspect</methodname> of the
 * <interfacename>Introspectable</interfacename> D-Bus interface, the
 * <emphasis>tracker</emphasis> object returns following informations:
 *
 * <programlisting>
 *   <command>
 *     dbus-send \
 *       --session \
 *       --type=method_call \
 *       --print-reply \
 *       --dest=org.caja-actions.DBus \
 *         /org/caja_actions/DBus/Tracker \
 *           org.freedesktop.DBus.Introspectable.Introspect
 *   </command>
 *   <computeroutput>
 *    <![CDATA[
 *     method return sender=:1.29 -> dest=:1.145 reply_serial=2
 *       string "<!DOCTYPE node PUBLIC "-//freedesktop//DTD D-BUS Object Introspection 1.0//EN"
 *     "http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd">
 *     <node>
 *       <interface name="org.freedesktop.DBus.Introspectable">
 *         <method name="Introspect">
 *           <arg name="data" direction="out" type="s"/>
 *         </method>
 *       </interface>
 *       <interface name="org.freedesktop.DBus.Properties">
 *         <method name="Get">
 *           <arg name="interface" direction="in" type="s"/>
 *           <arg name="propname" direction="in" type="s"/>
 *           <arg name="value" direction="out" type="v"/>
 *         </method>
 *         <method name="Set">
 *           <arg name="interface" direction="in" type="s"/>
 *           <arg name="propname" direction="in" type="s"/>
 *           <arg name="value" direction="in" type="v"/>
 *         </method>
 *         <method name="GetAll">
 *           <arg name="interface" direction="in" type="s"/>
 *           <arg name="props" direction="out" type="a{sv}"/>
 *         </method>
 *       </interface>
 *       <interface name="org.caja_actions.DBus.Tracker.Status">
 *         <method name="GetSelectedPaths">
 *           <arg name="paths" type="as" direction="out"/>
 *         </method>
 *       </interface>
 *     </node>
 *    ]]>
 *   </computeroutput>
 * </programlisting>
 */
#define CAJA_ACTIONS_DBUS_TRACKER_PATH       "/org/caja_actions/DBus/Tracker"

/**
 * CAJA_ACTIONS_DBUS_TRACKER_IFACE:
 *
 * An interface defined on the <emphasis>tracker</emphasis> object,
 * identified by its %CAJA_ACTIONS_DBUS_TRACKER_PATH D-Bus path.
 */
#define CAJA_ACTIONS_DBUS_TRACKER_IFACE  	"org.caja_actions.DBus.Tracker.Properties1"

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_DBUS_H__ */
