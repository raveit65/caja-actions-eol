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

#ifndef __NACT_ASSISTANT_IMPORT_H__
#define __NACT_ASSISTANT_IMPORT_H__

/**
 * SECTION: nact_assistant_import
 * @short_description: #NactAssistantImport class definition.
 * @include: nact/nact-assistant-import.h
 */

#include "base-assistant.h"

G_BEGIN_DECLS

#define NACT_ASSISTANT_IMPORT_TYPE					( nact_assistant_import_get_type())
#define NACT_ASSISTANT_IMPORT( object )				( G_TYPE_CHECK_INSTANCE_CAST( object, NACT_ASSISTANT_IMPORT_TYPE, NactAssistantImport ))
#define NACT_ASSISTANT_IMPORT_CLASS( klass )		( G_TYPE_CHECK_CLASS_CAST( klass, NACT_ASSISTANT_IMPORT_TYPE, NactAssistantImportClass ))
#define NACT_IS_ASSISTANT_IMPORT( object )			( G_TYPE_CHECK_INSTANCE_TYPE( object, NACT_ASSISTANT_IMPORT_TYPE ))
#define NACT_IS_ASSISTANT_IMPORT_CLASS( klass )		( G_TYPE_CHECK_CLASS_TYPE(( klass ), NACT_ASSISTANT_IMPORT_TYPE ))
#define NACT_ASSISTANT_IMPORT_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), NACT_ASSISTANT_IMPORT_TYPE, NactAssistantImportClass ))

typedef struct NactAssistantImportPrivate NactAssistantImportPrivate;

typedef struct {
	BaseAssistant               parent;
	NactAssistantImportPrivate *private;
}
	NactAssistantImport;

typedef struct NactAssistantImportClassPrivate NactAssistantImportClassPrivate;

typedef struct {
	BaseAssistantClass               parent;
	NactAssistantImportClassPrivate *private;
}
	NactAssistantImportClass;

GType nact_assistant_import_get_type( void );

void  nact_assistant_import_run( BaseWindow *main );

G_END_DECLS

#endif /* __NACT_ASSISTANT_IMPORT_H__ */
