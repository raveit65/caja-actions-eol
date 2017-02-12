/*
 * Caja Actions
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

#ifndef __CACT_ICON_CHOOSER_H__
#define __CACT_ICON_CHOOSER_H__

/**
 * SECTION: cact_icon_chooser
 * @short_description: CactIconChooser dialog box
 * @include: cact/cact-icon-chooser.h
 *
 * This class is derived from BaseDialog.
 */

#include "base-dialog.h"

G_BEGIN_DECLS

#define CACT_TYPE_ICON_CHOOSER                ( cact_icon_chooser_get_type())
#define CACT_ICON_CHOOSER( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, CACT_TYPE_ICON_CHOOSER, CactIconChooser ))
#define CACT_ICON_CHOOSER_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, CACT_TYPE_ICON_CHOOSER, CactIconChooserClass ))
#define CACT_IS_ICON_CHOOSER( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, CACT_TYPE_ICON_CHOOSER ))
#define CACT_IS_ICON_CHOOSER_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), CACT_TYPE_ICON_CHOOSER ))
#define CACT_ICON_CHOOSER_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), CACT_TYPE_ICON_CHOOSER, CactIconChooserClass ))

typedef struct _CactIconChooserPrivate        CactIconChooserPrivate;

typedef struct {
	/*< private >*/
	BaseDialog              parent;
	CactIconChooserPrivate *private;
}
	CactIconChooser;

typedef struct _CactIconChooserClassPrivate   CactIconChooserClassPrivate;

typedef struct {
	/*< private >*/
	BaseDialogClass              parent;
	CactIconChooserClassPrivate *private;
}
	CactIconChooserClass;

GType  cact_icon_chooser_get_type( void );

gchar *cact_icon_chooser_choose_icon( BaseWindow *main_window, const gchar *icon_name );

G_END_DECLS

#endif /* __CACT_ICON_CHOOSER_H__ */
