/*
 * Caja-Actions
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <api/na-timeout.h>

static gboolean on_timeout_event_timeout( NATimeout *timeout );
static gulong   time_val_diff( const GTimeVal *recent, const GTimeVal *old );

/**
 * na_timeout_event:
 * @timeout: the #NATimeout structure which will handle this event.
 */
void
na_timeout_event( NATimeout *event )
{
	g_return_if_fail( event != NULL );

	g_get_current_time( &event->last_time );

	if( !event->source_id ){
		event->source_id = g_timeout_add( event->timeout, ( GSourceFunc ) on_timeout_event_timeout, event );
	}
}

/*
 * this timer is set when we receive the first event of a serie
 * we continue to loop until last event is older that our burst timeout
 */
static gboolean
on_timeout_event_timeout( NATimeout *timeout )
{
	GTimeVal now;
	gulong diff;
	gulong timeout_usec;

	g_get_current_time( &now );
	diff = time_val_diff( &now, &timeout->last_time );
	timeout_usec = 1000*timeout->timeout;

	if( diff < timeout_usec ){
		/* do not stop */
		return( TRUE );
	}

	/* last individual notification is older that the 'timeout' parameter
	 * we may so suppose that the burst is terminated
	 * and feel authorized to trigger the defined callback
	 */
	( *timeout->handler )( timeout->user_data );

	/* at the end of the callback execution, reset the event source id.
	 * and stop the execution of this one
	 */
	timeout->source_id = 0;
	return( FALSE );
}

/*
 * returns the difference in microseconds.
 */
static gulong
time_val_diff( const GTimeVal *recent, const GTimeVal *old )
{
	gulong microsec = 1000000 * ( recent->tv_sec - old->tv_sec );
	microsec += recent->tv_usec  - old->tv_usec;
	return( microsec );
}
