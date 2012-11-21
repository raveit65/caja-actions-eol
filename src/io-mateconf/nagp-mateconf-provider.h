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

#ifndef __NAGP_MATECONF_PROVIDER_H__
#define __NAGP_MATECONF_PROVIDER_H__

/**
 * SECTION: nagp_mateconf_provider
 * @short_description: #NagpMateConfProvider class definition.
 * @include: na-mateconf-provider.h
 *
 * This class manages the MateConf I/O storage subsystem, or, in other words,
 * the MateConf subsystem as an #NAIIOProvider. As this, it should only be
 * used through the #NAIIOProvider interface.
 *
 * #NagpMateConfProvider uses #NAMateConfMonitor to watch at the configuration
 * tree. Modifications are notified to the #NAIIOProvider interface.
 */

#include <glib-object.h>
#include <mateconf/mateconf-client.h>

G_BEGIN_DECLS

#define NAGP_MATECONF_PROVIDER_TYPE				( nagp_mateconf_provider_get_type())
#define NAGP_MATECONF_PROVIDER( object )			( G_TYPE_CHECK_INSTANCE_CAST( object, NAGP_MATECONF_PROVIDER_TYPE, NagpMateConfProvider ))
#define NAGP_MATECONF_PROVIDER_CLASS( klass )		( G_TYPE_CHECK_CLASS_CAST( klass, NAGP_MATECONF_PROVIDER_TYPE, NagpMateConfProviderClass ))
#define NAGP_IS_MATECONF_PROVIDER( object )		( G_TYPE_CHECK_INSTANCE_TYPE( object, NAGP_MATECONF_PROVIDER_TYPE ))
#define NAGP_IS_MATECONF_PROVIDER_CLASS( klass )	( G_TYPE_CHECK_CLASS_TYPE(( klass ), NAGP_MATECONF_PROVIDER_TYPE ))
#define NAGP_MATECONF_PROVIDER_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), NAGP_MATECONF_PROVIDER_TYPE, NagpMateConfProviderClass ))

/* private instance data
 */
typedef struct _NagpMateConfProviderPrivate {
	/*< private >*/
	gboolean     dispose_has_run;
	MateConfClient *mateconf;
	GList       *monitors;
	guint        event_source_id;
	GTimeVal     last_event;
}
	NagpMateConfProviderPrivate;

typedef struct {
	/*< private >*/
	GObject                   parent;
	NagpMateConfProviderPrivate *private;
}
	NagpMateConfProvider;

typedef struct _NagpMateConfProviderClassPrivate   NagpMateConfProviderClassPrivate;

typedef struct {
	/*< private >*/
	GObjectClass                   parent;
	NagpMateConfProviderClassPrivate *private;
}
	NagpMateConfProviderClass;

GType nagp_mateconf_provider_get_type     ( void );
void  nagp_mateconf_provider_register_type( GTypeModule *module );

G_END_DECLS

#endif /* __NAGP_MATECONF_PROVIDER_H__ */
