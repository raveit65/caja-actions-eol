/*
 * Caja Actions
 * A Caja extension which offers configurable context menu actions.
 *
 * Copyright (C) 2005 The MATE Foundation
 * Copyright (C) 2006, 2007, 2008 Frederic Ruaudel and others (see AUTHORS)
 * Copyright (C) 2009, 2010 Pierre Wieser and others (see AUTHORS)
 *
 * This Program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This Program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this Library; see the file COPYING.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA 02111-1307, USA.
 *
 * Authors:
 *   Frederic Ruaudel <grumz@grumz.net>
 *   Rodrigo Moya <rodrigo@mate-db.org>
 *   Pierre Wieser <pwieser@trychlos.org>
 *   ... and many others (see AUTHORS)
 */

#ifndef __CAJA_ACTIONS_API_NA_EXTENSION_H__
#define __CAJA_ACTIONS_API_NA_EXTENSION_H__

/**
 * SECTION: na_extension
 * @short_description: Caja-Actions extension interface definition.
 * @include: caja-actions/na-extension.h
 *
 * Caja-Actions accepts extensions as dynamically loadable libraries
 * (aka plugins).
 *
 * At startup time, candidate libraries are searched for in PKGLIBDIR/
 * directory. A valid candidate must at least export the #na_extension_startup()
 * and #na_extension_list_types() functions.
 *
 * Caja-Actions v 2.30 - API version:  1
 */

#include <glib-object.h>

G_BEGIN_DECLS

/**
 * na_extension_startup:
 * @module: the #GTypeModule of the library being loaded.
 *
 * This function is called by the Caja-Actions plugin manager when
 * the library is first loaded in memory. The library may so take
 * advantage of this call by initializing itself, registering its
 * internal #GType types, etc.
 *
 * Returns: %TRUE if the initialization is successfull, %FALSE else.
 * In this later case, the library is unloaded and no more considered.
 *
 * A Caja-Actions extension must implement this function in order
 * to be considered as a valid candidate to dynamic load.
 */
gboolean na_extension_startup    ( GTypeModule *module );

/**
 * na_extension_get_version:
 *
 * Returns: the version of this API supported by the module.
 *
 * If this function is not exported by the library, or returns zero,
 * the plugin manager considers that the library only implements the
 * version 1 of this API.
 */
guint    na_extension_get_version( void );

/**
 * na_extension_list_types:
 * @types: the address where to store the zero-terminated array of
 *  instantiable #GType types this library implements.
 *
 * Returned #GType types must already have been registered in the
 * #GType system (e.g. at #na_extension_startup() time), and may implement
 * one or more of the interfaces defined in Caja-Actions API.
 *
 * The Caja-Actions plugin manager will instantiate one #GTypeInstance-
 * derived object for each returned #GType type, and associate these objects
 * to this library.
 *
 * Returns: the number of #GType types returned in the @types array, not
 * counting the terminating zero item.
 *
 * A Caja-Actions extension must implement this function in order
 * to be considered as a valid candidate to dynamic load.
 *
 * If this function is not exported by the library, or returns zero,
 * the plugin manager considers that the library doesn't implement
 * any Caja-Action interface. In this case, the library is
 * unloaded and no more considered.
 */
guint    na_extension_list_types ( const GType **types );

/**
 * na_extension_shutdown:
 *
 * This function is called by Caja-Actions when it is about to
 * shutdown itself.
 *
 * The dynamicaly loaded library may take advantage of this call to
 * release any resource it may have previously allocated.
 */
void     na_extension_shutdown   ( void );

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_EXTENSION_H__ */
