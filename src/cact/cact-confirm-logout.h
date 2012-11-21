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

#ifndef __CACT_CONFIRM_LOGOUT_H__
#define __CACT_CONFIRM_LOGOUT_H__

/**
 * SECTION: cact_preferences_editor
 * @short_description: #CactConfirmLogout class definition.
 * @include: cact/cact-preferences-editor.h
 *
 * This class is derived from CactWindow.
 * It encapsulates the "PreferencesDialog" widget dialog.
 */

#include "base-dialog.h"
#include "cact-main-window.h"

G_BEGIN_DECLS

#define CACT_TYPE_CONFIRM_LOGOUT                ( cact_confirm_logout_get_type())
#define CACT_CONFIRM_LOGOUT( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, CACT_TYPE_CONFIRM_LOGOUT, CactConfirmLogout ))
#define CACT_CONFIRM_LOGOUT_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, CACT_TYPE_CONFIRM_LOGOUT, CactConfirmLogoutClass ))
#define CACT_IS_CONFIRM_LOGOUT( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, CACT_TYPE_CONFIRM_LOGOUT ))
#define CACT_IS_CONFIRM_LOGOUT_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), CACT_TYPE_CONFIRM_LOGOUT ))
#define CACT_CONFIRM_LOGOUT_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), CACT_TYPE_CONFIRM_LOGOUT, CactConfirmLogoutClass ))

typedef struct _CactConfirmLogoutPrivate        CactConfirmLogoutPrivate;

typedef struct {
	/*< private >*/
	BaseDialog                parent;
	CactConfirmLogoutPrivate *private;
}
	CactConfirmLogout;

typedef struct _CactConfirmLogoutClassPrivate   CactConfirmLogoutClassPrivate;

typedef struct {
	/*< private >*/
	BaseDialogClass                parent;
	CactConfirmLogoutClassPrivate *private;
}
	CactConfirmLogoutClass;

GType    cact_confirm_logout_get_type( void );

gboolean cact_confirm_logout_run( CactMainWindow *parent );

G_END_DECLS

#endif /* __CACT_CONFIRM_LOGOUT_H__ */
