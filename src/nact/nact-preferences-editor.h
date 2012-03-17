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

#ifndef __NACT_PREFERENCES_EDITOR_H__
#define __NACT_PREFERENCES_EDITOR_H__

/**
 * SECTION: nact_preferences_editor
 * @short_description: #NactPreferencesEditor class definition.
 * @include: nact/nact-preferences-editor.h
 *
 * This class is derived from NactWindow.
 * It encapsulates the "PreferencesDialog" widget dialog.
 */

#include "base-dialog.h"

G_BEGIN_DECLS

#define NACT_PREFERENCES_EDITOR_TYPE				( nact_preferences_editor_get_type())
#define NACT_PREFERENCES_EDITOR( object )			( G_TYPE_CHECK_INSTANCE_CAST( object, NACT_PREFERENCES_EDITOR_TYPE, NactPreferencesEditor ))
#define NACT_PREFERENCES_EDITOR_CLASS( klass )		( G_TYPE_CHECK_CLASS_CAST( klass, NACT_PREFERENCES_EDITOR_TYPE, NactPreferencesEditorClass ))
#define NACT_IS_PREFERENCES_EDITOR( object )		( G_TYPE_CHECK_INSTANCE_TYPE( object, NACT_PREFERENCES_EDITOR_TYPE ))
#define NACT_IS_PREFERENCES_EDITOR_CLASS( klass )	( G_TYPE_CHECK_CLASS_TYPE(( klass ), NACT_PREFERENCES_EDITOR_TYPE ))
#define NACT_PREFERENCES_EDITOR_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), NACT_PREFERENCES_EDITOR_TYPE, NactPreferencesEditorClass ))

typedef struct NactPreferencesEditorPrivate NactPreferencesEditorPrivate;

typedef struct {
	BaseDialog                    parent;
	NactPreferencesEditorPrivate *private;
}
	NactPreferencesEditor;

typedef struct NactPreferencesEditorClassPrivate NactPreferencesEditorClassPrivate;

typedef struct {
	BaseDialogClass                    parent;
	NactPreferencesEditorClassPrivate *private;
}
	NactPreferencesEditorClass;

GType nact_preferences_editor_get_type( void );

void  nact_preferences_editor_run( BaseWindow *parent );

G_END_DECLS

#endif /* __NACT_PREFERENCES_EDITOR_H__ */
