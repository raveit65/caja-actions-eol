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

#ifndef __CACT_ASSISTANT_IMPORT_H__
#define __CACT_ASSISTANT_IMPORT_H__

/**
 * SECTION: cact_assistant_import
 * @short_description: #CactAssistantImport class definition.
 * @include: cact/cact-assistant-import.h
 */

#include "base-assistant.h"

G_BEGIN_DECLS

#define CACT_TYPE_ASSISTANT_IMPORT                ( cact_assistant_import_get_type())
#define CACT_ASSISTANT_IMPORT( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, CACT_TYPE_ASSISTANT_IMPORT, CactAssistantImport ))
#define CACT_ASSISTANT_IMPORT_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, CACT_TYPE_ASSISTANT_IMPORT, CactAssistantImportClass ))
#define CACT_IS_ASSISTANT_IMPORT( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, CACT_TYPE_ASSISTANT_IMPORT ))
#define CACT_IS_ASSISTANT_IMPORT_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), CACT_TYPE_ASSISTANT_IMPORT ))
#define CACT_ASSISTANT_IMPORT_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), CACT_TYPE_ASSISTANT_IMPORT, CactAssistantImportClass ))

typedef struct _CactAssistantImportPrivate        CactAssistantImportPrivate;

typedef struct {
	/*< private >*/
	BaseAssistant               parent;
	CactAssistantImportPrivate *private;
}
	CactAssistantImport;

typedef struct _CactAssistantImportClassPrivate   CactAssistantImportClassPrivate;

typedef struct {
	/*< private >*/
	BaseAssistantClass               parent;
	CactAssistantImportClassPrivate *private;
}
	CactAssistantImportClass;

GType cact_assistant_import_get_type( void );

void  cact_assistant_import_run( BaseWindow *main );

G_END_DECLS

#endif /* __CACT_ASSISTANT_IMPORT_H__ */
