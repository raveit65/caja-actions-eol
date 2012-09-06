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

#ifndef __BASE_DIALOG_H__
#define __BASE_DIALOG_H__

/**
 * SECTION: base_dialog
 * @short_description: #BaseDialog class definition.
 * @include: cact/base-dialog.h
 *
 * This class is derived from BaseWindow class, and serves as a base
 * class for all Caja Actions dialogs.
 */

#include "base-window.h"

G_BEGIN_DECLS

#define BASE_DIALOG_TYPE				( base_dialog_get_type())
#define BASE_DIALOG( object )			( G_TYPE_CHECK_INSTANCE_CAST( object, BASE_DIALOG_TYPE, BaseDialog ))
#define BASE_DIALOG_CLASS( klass )		( G_TYPE_CHECK_CLASS_CAST( klass, BASE_DIALOG_TYPE, BaseDialogClass ))
#define BASE_IS_DIALOG( object )		( G_TYPE_CHECK_INSTANCE_TYPE( object, BASE_DIALOG_TYPE ))
#define BASE_IS_DIALOG_CLASS( klass )	( G_TYPE_CHECK_CLASS_TYPE(( klass ), BASE_DIALOG_TYPE ))
#define BASE_DIALOG_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), BASE_DIALOG_TYPE, BaseDialogClass ))

typedef struct BaseDialogPrivate      BaseDialogPrivate;

typedef struct {
	BaseWindow         parent;
	BaseDialogPrivate *private;
}
	BaseDialog;

typedef struct BaseDialogClassPrivate BaseDialogClassPrivate;

typedef struct {
	BaseWindowClass         parent;
	BaseDialogClassPrivate *private;
}
	BaseDialogClass;

GType base_dialog_get_type( void );

G_END_DECLS

#endif /* __BASE_DIALOG_H__ */
