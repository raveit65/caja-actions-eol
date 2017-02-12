/*
 * Caja-Actions
 * A Caja extension which offers configurable context menu actions.
 *
 * Copyright (C) 2005 The GNOME Foundation
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

#ifndef __CACT_ADD_CAPABILITY_DIALOG_H__
#define __CACT_ADD_CAPABILITY_DIALOG_H__

/**
 * SECTION: cact_add_capability_dialog
 * @short_description: #CactAddCapabilityDialog class definition.
 * @include: cact/cact-add-capability-dialog.h
 *
 * The dialog let the user pick a capability from the default list
 * when adding a new capability to a profile (resp. an action, a menu).
 */

#include "base-dialog.h"

G_BEGIN_DECLS

#define CACT_TYPE_ADD_CAPABILITY_DIALOG                ( cact_add_capability_dialog_get_type())
#define CACT_ADD_CAPABILITY_DIALOG( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, CACT_TYPE_ADD_CAPABILITY_DIALOG, CactAddCapabilityDialog ))
#define CACT_ADD_CAPABILITY_DIALOG_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, CACT_TYPE_ADD_CAPABILITY_DIALOG, CactAddCapabilityDialogClass ))
#define CACT_IS_ADD_CAPABILITY_DIALOG( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, CACT_TYPE_ADD_CAPABILITY_DIALOG ))
#define CACT_IS_ADD_CAPABILITY_DIALOG_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), CACT_TYPE_ADD_CAPABILITY_DIALOG ))
#define CACT_ADD_CAPABILITY_DIALOG_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), CACT_TYPE_ADD_CAPABILITY_DIALOG, CactAddCapabilityDialogClass ))

typedef struct _CactAddCapabilityDialogPrivate         CactAddCapabilityDialogPrivate;

typedef struct {
	/*< private >*/
	BaseDialog                      parent;
	CactAddCapabilityDialogPrivate *private;
}
	CactAddCapabilityDialog;

typedef struct _CactAddCapabilityDialogClassPrivate    CactAddCapabilityDialogClassPrivate;

typedef struct {
	/*< private >*/
	BaseDialogClass                      parent;
	CactAddCapabilityDialogClassPrivate *private;
}
	CactAddCapabilityDialogClass;

GType  cact_add_capability_dialog_get_type( void );

gchar *cact_add_capability_dialog_run( BaseWindow *parent, GSList *capabilities );

G_END_DECLS

#endif /* __CACT_ADD_CAPABILITY_DIALOG_H__ */
