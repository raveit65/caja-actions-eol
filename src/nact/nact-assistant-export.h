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

#ifndef __NACT_ASSISTANT_EXPORT_H__
#define __NACT_ASSISTANT_EXPORT_H__

/**
 * SECTION: nact_assistant_export
 * @short_description: #NactAssistantExport class definition.
 * @include: nact/nact-assistant-export.h
 *
 * Rationale:
 *
 * Up to v 1.10.x, actions are exported as config_uuid.schemas. These
 * are actually full MateConf schema exports of the form :
 *  <schemalist>
 *   <schema>
 *    <key>/schemas/apps/caja-actions/..../uuid/label</key>
 *    <applyto>/apps/..../label</applyto>
 *
 * I don't know why Frederic had choosen to export as schema. But this
 * implies that :
 * - if all actions are imported via mateconftool-2 --install-schema-file,
 *   then the schema will be repeated once for each imported action (as
 *   schema is attached here to the uuid), which is obviously a waste
 *   of resources
 * - it seems that MateConfClient refuses to delete only the 'applyto' key
 *   when there is a corresponding schema key ..? (to be confirmed)
 *
 * Considering exporting schemas, we have two goals with this :
 * - make the files lighter
 * - keep the compatibility with previous mode
 *
 * which means exporting the minimal schema file which can be imported
 * with v1.10 and previous series, and via mateconftool-2 --install-schema-file.
 */

#include "base-assistant.h"

G_BEGIN_DECLS

#define NACT_ASSISTANT_EXPORT_TYPE					( nact_assistant_export_get_type())
#define NACT_ASSISTANT_EXPORT( object )				( G_TYPE_CHECK_INSTANCE_CAST( object, NACT_ASSISTANT_EXPORT_TYPE, NactAssistantExport ))
#define NACT_ASSISTANT_EXPORT_CLASS( klass )		( G_TYPE_CHECK_CLASS_CAST( klass, NACT_ASSISTANT_EXPORT_TYPE, NactAssistantExportClass ))
#define NACT_IS_ASSISTANT_EXPORT( object )			( G_TYPE_CHECK_INSTANCE_TYPE( object, NACT_ASSISTANT_EXPORT_TYPE ))
#define NACT_IS_ASSISTANT_EXPORT_CLASS( klass )		( G_TYPE_CHECK_CLASS_TYPE(( klass ), NACT_ASSISTANT_EXPORT_TYPE ))
#define NACT_ASSISTANT_EXPORT_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), NACT_ASSISTANT_EXPORT_TYPE, NactAssistantExportClass ))

typedef struct NactAssistantExportPrivate      NactAssistantExportPrivate;

typedef struct {
	BaseAssistant               parent;
	NactAssistantExportPrivate *private;
}
	NactAssistantExport;

typedef struct NactAssistantExportClassPrivate NactAssistantExportClassPrivate;

typedef struct {
	BaseAssistantClass               parent;
	NactAssistantExportClassPrivate *private;
}
	NactAssistantExportClass;

GType nact_assistant_export_get_type( void );

void  nact_assistant_export_run( BaseWindow *parent );

G_END_DECLS

#endif /* __NACT_ASSISTANT_EXPORT_H__ */
