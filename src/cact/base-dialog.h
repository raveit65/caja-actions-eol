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

#ifndef __BASE_DIALOG_H__
#define __BASE_DIALOG_H__

/**
 * SECTION: base-dialog
 * @title: BaseDialog
 * @short_description: The BaseDialog dialog base class definition
 * @include: base-dialog.h
 *
 * This class is derived from BaseWindow class, and serves as a base
 * class for all Caja-Actions dialogs.
 */

#include "base-window.h"

G_BEGIN_DECLS

#define BASE_TYPE_DIALOG                ( base_dialog_get_type())
#define BASE_DIALOG( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, BASE_TYPE_DIALOG, BaseDialog ))
#define BASE_DIALOG_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, BASE_TYPE_DIALOG, BaseDialogClass ))
#define BASE_IS_DIALOG( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, BASE_TYPE_DIALOG ))
#define BASE_IS_DIALOG_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), BASE_TYPE_DIALOG ))
#define BASE_DIALOG_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), BASE_TYPE_DIALOG, BaseDialogClass ))

typedef struct _BaseDialogPrivate       BaseDialogPrivate;

typedef struct {
	/*< private >*/
	BaseWindow         parent;
	BaseDialogPrivate *private;
}
	BaseDialog;

typedef struct _BaseDialogClassPrivate  BaseDialogClassPrivate;

/**
 * BaseDialogClass:
 * @cancel: the dialog box is not validated.
 * @ok:     the dialog box is validated.
 *
 * This defines the virtual method a derived class may, should or must implement.
 */
typedef struct {
	/*< private >*/
	BaseWindowClass         parent;
	BaseDialogClassPrivate *private;

	/*< public >*/
	/**
	 * cancel:
	 * @dialog: this #BaseDialog instance.
	 *
	 * Invoked when the dialog box is closed without having been validated.
	 */
	void ( *cancel )( BaseDialog *dialog );

	/**
	 * ok:
	 * @dialog: this #BaseDialog instance.
	 *
	 * Invoked when the dialog box is validated.
	 */
	void ( *ok )    ( BaseDialog *dialog );
}
	BaseDialogClass;

GType base_dialog_get_type( void );

G_END_DECLS

#endif /* __BASE_DIALOG_H__ */
