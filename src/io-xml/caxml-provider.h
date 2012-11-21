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

#ifndef __CAXML_PROVIDER_H__
#define __CAXML_PROVIDER_H__

/**
 * SECTION: caxml_provider
 * @short_description: #CAXMLProvider class definition.
 * @include: caxml-provider.h
 *
 * This class manages I/O in XML formats.
 */

#include <glib-object.h>

G_BEGIN_DECLS

#define CAXML_TYPE_PROVIDER                ( caxml_provider_get_type())
#define CAXML_PROVIDER( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, CAXML_TYPE_PROVIDER, CAXMLProvider ))
#define CAXML_PROVIDER_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, CAXML_TYPE_PROVIDER, CAXMLProviderClass ))
#define NA_IS_XML_PROVIDER( object )       ( G_TYPE_CHECK_INSTANCE_TYPE( object, CAXML_TYPE_PROVIDER ))
#define NA_IS_XML_PROVIDER_CLASS( klass )  ( G_TYPE_CHECK_CLASS_TYPE(( klass ), CAXML_TYPE_PROVIDER ))
#define CAXML_PROVIDER_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), CAXML_TYPE_PROVIDER, CAXMLProviderClass ))

typedef struct _CAXMLProviderPrivate       CAXMLProviderPrivate;

typedef struct {
	/*< private >*/
	GObject               parent;
	CAXMLProviderPrivate *private;
}
	CAXMLProvider;

typedef struct _CAXMLProviderClassPrivate  CAXMLProviderClassPrivate;

typedef struct {
	/*< private >*/
	GObjectClass               parent;
	CAXMLProviderClassPrivate *private;
}
	CAXMLProviderClass;

GType caxml_provider_get_type     ( void );
void  caxml_provider_register_type( GTypeModule *module );

G_END_DECLS

#endif /* __CAXML_PROVIDER_H__ */
