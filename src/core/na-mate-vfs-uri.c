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

/*
 * pwi 2009-07-29
 * shamelessly pull out of MateVFS (mate-vfs-uri and consorts)
 */

/* mate-vfs-uri.h - URI handling for the MATE Virtual File System.

   Copyright (C) 1999 Free Software Foundation

   The Mate Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Mate Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Mate Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Ettore Perazzoli <ettore@comm2000.it> */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include "na-mate-vfs-uri.h"

#define HEX_ESCAPE '%'

static void         collapse_slash_runs (char *path, int from_offset);
static int          find_next_slash (const char *path, int current_offset);
static int          find_slash_before_offset (const char *path, int to);
static const gchar *get_method_string (const gchar *substring, gchar **method_string);
static gchar *      mate_vfs_canonicalize_pathname (gchar *path);
static char        *mate_vfs_escape_set(const char *string, const char *match_set);
static void         mate_vfs_remove_optional_escapes (char *uri);
static char *       mate_vfs_unescape_string (const gchar *escaped_string, const gchar *illegal_characters);
static int          hex_to_int (gchar c);
static void         set_uri_element (NAMateVFSURI *vfs, const gchar *text, guint len);
static gchar       *split_toplevel_uri (const gchar *path, guint path_len,
												gchar **host_return, gchar **user_return,
												guint *port_return, gchar **password_return);
static int          unescape_character (const char *scanner);

void
na_mate_vfs_uri_parse( NAMateVFSURI *vfs, const gchar *text_uri )
{
	const gchar *method_scanner;
	gchar *extension_scanner;

	vfs->path = NULL;
	vfs->scheme = NULL;
	vfs->host_name = NULL;
	vfs->host_port = 0;
	vfs->user_name = NULL;
	vfs->password = NULL;

	if (text_uri[0] == '\0') {
		return;
	}

	method_scanner = get_method_string(text_uri, &vfs->scheme );
	if (strcmp (vfs->scheme, "pipe") == 0 ){
		return;
	}

	extension_scanner = strchr (method_scanner, MATE_VFS_URI_MAGIC_CHR);
	if (extension_scanner == NULL) {
		set_uri_element (vfs, method_scanner, strlen (method_scanner));
		return;
	}

	/* handle '#' */
	set_uri_element (vfs, method_scanner, extension_scanner - method_scanner);

	if (strchr (extension_scanner, ':') == NULL) {
		/* extension is a fragment identifier */
		/*uri->fragment_id = g_strdup (extension_scanner + 1);*/
		return;
	}
}

void
na_mate_vfs_uri_free( NAMateVFSURI *vfs )
{
	g_free( vfs->path );
	g_free( vfs->scheme );
	g_free( vfs->host_name );
	g_free( vfs->user_name );
	g_free( vfs->password );
	g_free( vfs );
}

static void
collapse_slash_runs (char *path, int from_offset)
{
	int i;
	/* Collapse multiple `/'s in a row. */
	for (i = from_offset;; i++) {
		if (path[i] != MATE_VFS_URI_PATH_CHR) {
			break;
		}
	}

	if (from_offset < i) {
		memmove (path + from_offset, path + i, strlen (path + i) + 1);
		i = from_offset + 1;
	}
}

static int
find_next_slash (const char *path, int current_offset)
{
	const char *match;

	g_assert (current_offset <= strlen (path));

	match = strchr (path + current_offset, MATE_VFS_URI_PATH_CHR);
	return match == NULL ? -1 : match - path;
}

static int
find_slash_before_offset (const char *path, int to)
{
	int result;
	int next_offset;

	result = -1;
	next_offset = 0;
	for (;;) {
		next_offset = find_next_slash (path, next_offset);
		if (next_offset < 0 || next_offset >= to) {
			break;
		}
		result = next_offset;
		next_offset++;
	}
	return result;
}

static const gchar *
get_method_string (const gchar *substring, gchar **method_string)
{
	const gchar *p;
	char *method;

	for (p = substring;
	     g_ascii_isalnum (*p) || *p == '+' || *p == '-' || *p == '.';
	     p++)
		;

	if (*p == ':'
#ifdef G_OS_WIN32
	              &&
	    !(p == substring + 1 && g_ascii_isalpha (*substring))
#endif
								 ) {
		/* Found toplevel method specification.  */
		method = g_strndup (substring, p - substring);
		*method_string = g_ascii_strdown (method, -1);
		g_free (method);
		p++;
	} else {
		*method_string = g_strdup ("file");
		p = substring;
	}
	return p;
}

/* Canonicalize path, and return a new path.  Do everything in situ.  The new
   path differs from path in:

     Multiple `/'s are collapsed to a single `/'.
     Leading `./'s and trailing `/.'s are removed.
     Non-leading `../'s and trailing `..'s are handled by removing
     portions of the path.  */
static gchar *
mate_vfs_canonicalize_pathname (gchar *path)
{
	int i, marker;

	if (path == NULL || strlen (path) == 0) {
		return "";
	}

	/* Walk along path looking for things to compact. */
	for (i = 0, marker = 0;;) {
		if (!path[i])
			break;

		/* Check for `../', `./' or trailing `.' by itself. */
		if (path[i] == '.') {
			/* Handle trailing `.' by itself. */
			if (path[i + 1] == '\0') {
				if (i > 1 && path[i - 1] == MATE_VFS_URI_PATH_CHR) {
					/* strip the trailing /. */
					path[i - 1] = '\0';
				} else {
					/* convert path "/." to "/" */
					path[i] = '\0';
				}
				break;
			}

			/* Handle `./'. */
			if (path[i + 1] == MATE_VFS_URI_PATH_CHR) {
				memmove (path + i, path + i + 2,
					 strlen (path + i + 2) + 1);
				if (i == 0) {
					/* don't leave leading '/' for paths that started
					 * as relative (.//foo)
					 */
					collapse_slash_runs (path, i);
					marker = 0;
				}
				continue;
			}

			/* Handle `../' or trailing `..' by itself.
			 * Remove the previous xxx/ part
			 */
			if (path[i + 1] == '.'
			    && (path[i + 2] == MATE_VFS_URI_PATH_CHR
				|| path[i + 2] == '\0')) {

				/* ignore ../ at the beginning of a path */
				if (i != 0) {
					marker = find_slash_before_offset (path, i - 1);

					/* Either advance past '/' or point to the first character */
					marker ++;
					if (path [i + 2] == '\0' && marker > 1) {
						/* If we are looking at a /.. at the end of the uri and we
						 * need to eat the last '/' too.
						 */
						 marker--;
					}
					g_assert(marker < i);

					if (path[i + 2] == MATE_VFS_URI_PATH_CHR) {
						/* strip the entire ../ string */
						i++;
					}

					memmove (path + marker, path + i + 2,
						 strlen (path + i + 2) + 1);
					i = marker;
				} else {
					i = 2;
					if (path[i] == MATE_VFS_URI_PATH_CHR) {
						i++;
					}
				}
				collapse_slash_runs (path, i);
				continue;
			}
		}

		/* advance to the next '/' */
		i = find_next_slash (path, i);

		/* If we didn't find any slashes, then there is nothing left to do. */
		if (i < 0) {
			break;
		}

		marker = i++;
		collapse_slash_runs (path, i);
	}
	return path;
}

/*  Escape undesirable characters using %
 *  -------------------------------------
 *
 * This function takes a pointer to a string in which
 * some characters may be unacceptable unescaped.
 * It returns a string which has these characters
 * represented by a '%' character followed by two hex digits.
 *
 * This routine returns a g_malloced string.
 */

static const gchar hex[16] = "0123456789ABCDEF";

/*
 * mate_vfs_escape_set:
 * @string: string to be escaped.
 * @match_set: a string containing all characters to be escaped in @string.
 *
 * Escapes all characters in @string which are listed in @match_set.
 *
 * Return value: a newly allocated string equivalent to @string but
 * with characters in @match_string escaped.
 */
static char *
mate_vfs_escape_set (const char *string,
	              const char *match_set)
{
	char *result;
	const char *scanner;
	char *result_scanner;
	int escape_count;

	escape_count = 0;

	if (string == NULL) {
		return NULL;
	}

	if (match_set == NULL) {
		return g_strdup (string);
	}

	for (scanner = string; *scanner != '\0'; scanner++) {
		if (strchr(match_set, *scanner) != NULL) {
			/* this character is in the set of characters
			 * we want escaped.
			 */
			escape_count++;
		}
	}

	if (escape_count == 0) {
		return g_strdup (string);
	}

	/* allocate two extra characters for every character that
	 * needs escaping and space for a trailing zero
	 */
	result = g_malloc (scanner - string + escape_count * 2 + 1);
	for (scanner = string, result_scanner = result; *scanner != '\0'; scanner++) {
		if (strchr(match_set, *scanner) != NULL) {
			/* this character is in the set of characters
			 * we want escaped.
			 */
			*result_scanner++ = HEX_ESCAPE;
			*result_scanner++ = hex[*scanner >> 4];
			*result_scanner++ = hex[*scanner & 15];

		} else {
			*result_scanner++ = *scanner;
		}
	}

	*result_scanner = '\0';

	return result;
}

/*
 * mate_vfs_remove_optional_escapes:
 * @uri: an escaped uri.
 *
 * Scans the @uri and converts characters that do not have to be
 * escaped into an un-escaped form. The characters that get treated this
 * way are defined as unreserved by the RFC.
 *
 * Return value: an error value if the @uri is found to be malformed.
 */

enum {
	RESERVED = 1,
	UNRESERVED,
	DELIMITERS,
	UNWISE,
	CONTROL,
	SPACE
};

static const guchar uri_character_kind[128] =
{
    CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,
    CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,
    CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,
    CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,
    /* ' '        !          "          #          $          %          &          '      */
    SPACE     ,UNRESERVED,DELIMITERS,DELIMITERS,RESERVED  ,DELIMITERS,RESERVED  ,UNRESERVED,
    /*  (         )          *          +          ,          -          .          /      */
    UNRESERVED,UNRESERVED,UNRESERVED,RESERVED  ,RESERVED  ,UNRESERVED,UNRESERVED,RESERVED  ,
    /*  0         1          2          3          4          5          6          7      */
    UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,
    /*  8         9          :          ;          <          =          >          ?      */
    UNRESERVED,UNRESERVED,RESERVED  ,RESERVED  ,DELIMITERS,RESERVED  ,DELIMITERS,RESERVED  ,
    /*  @         A          B          C          D          E          F          G      */
    RESERVED  ,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,
    /*  H         I          J          K          L          M          N          O      */
    UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,
    /*  P         Q          R          S          T          U          V          W      */
    UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,
    /*  X         Y          Z          [          \          ]          ^          _      */
    UNRESERVED,UNRESERVED,UNRESERVED,UNWISE    ,UNWISE    ,UNWISE    ,UNWISE    ,UNRESERVED,
    /*  `         a          b          c          d          e          f          g      */
    UNWISE    ,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,
    /*  h         i          j          k          l          m          n          o      */
    UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,
    /*  p         q          r          s          t          u          v          w      */
    UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,
    /*  x         y          z         {           |          }          ~         DEL     */
    UNRESERVED,UNRESERVED,UNRESERVED,UNWISE    ,UNWISE    ,UNWISE    ,UNRESERVED,CONTROL
};

static void
mate_vfs_remove_optional_escapes (char *uri)
{
	guchar *scanner;
	int character;
	int length;

	if (uri == NULL) {
		return;
	}

	length = strlen (uri);

	for (scanner = (guchar *)uri; *scanner != '\0'; scanner++, length--) {
		if (*scanner == HEX_ESCAPE) {
			character = unescape_character ((char *)scanner + 1);
			if (character < 0) {
				/* invalid hexadecimal character */
				return;
			}

			if (uri_character_kind [character] == UNRESERVED) {
				/* This character does not need to be escaped, convert it
				 * to a non-escaped form.
				 */
				*scanner = (guchar)character;
				g_assert (length >= 3);

				/* Shrink the string covering up the two extra digits of the
				 * escaped character. Include the trailing '\0' in the copy
				 * to keep the string terminated.
				 */
				memmove (scanner + 1, scanner + 3, length - 2);
			} else {
				/* This character must stay escaped, skip the entire
				 * escaped sequence
				 */
				scanner += 2;
			}
			length -= 2;

		} else if (*scanner > 127
			|| uri_character_kind [*scanner] == DELIMITERS
			|| uri_character_kind [*scanner] == UNWISE
			|| uri_character_kind [*scanner] == CONTROL) {
			/* It is illegal for this character to be in an un-escaped form
			 * in the uri.
			 */
			return;
		}
	}
}

static int
hex_to_int (gchar c)
{
	return  c >= '0' && c <= '9' ? c - '0'
		: c >= 'A' && c <= 'F' ? c - 'A' + 10
		: c >= 'a' && c <= 'f' ? c - 'a' + 10
		: -1;
}

static int
unescape_character (const char *scanner)
{
	int first_digit;
	int second_digit;

	first_digit = hex_to_int (*scanner++);
	if (first_digit < 0) {
		return -1;
	}

	second_digit = hex_to_int (*scanner++);
	if (second_digit < 0) {
		return -1;
	}

	return (first_digit << 4) | second_digit;
}

/*
 * mate_vfs_unescape_string:
 * @escaped_string: an escaped uri, path, or other string.
 * @illegal_characters: a string containing a sequence of characters
 * considered "illegal" to be escaped, '\0' is automatically in this list.
 *
 * Decodes escaped characters (i.e. PERCENTxx sequences) in @escaped_string.
 * Characters are encoded in PERCENTxy form, where xy is the ASCII hex code
 * for character 16x+y.
 *
 * Return value: a newly allocated string with the unescaped
 * equivalents, or %NULL if @escaped_string contained an escaped
 * encoding of one of the characters in @illegal_characters.
 */
static char *
mate_vfs_unescape_string (const gchar *escaped_string,
			   const gchar *illegal_characters)
{
	const gchar *in;
	gchar *out, *result;
	gint character;

	if (escaped_string == NULL) {
		return NULL;
	}

	result = g_malloc (strlen (escaped_string) + 1);

	out = result;
	for (in = escaped_string; *in != '\0'; in++) {
		character = *in;
		if (*in == HEX_ESCAPE) {
			character = unescape_character (in + 1);

			/* Check for an illegal character. We consider '\0' illegal here. */
			if (character <= 0
			    || (illegal_characters != NULL
				&& strchr (illegal_characters, (char)character) != NULL)) {
				g_free (result);
				return NULL;
			}
			in += 2;
		}
		*out++ = (char)character;
	}

	*out = '\0';
	g_assert (out - result <= strlen (escaped_string));
	return result;

}

static void
set_uri_element (NAMateVFSURI *vfs,
		 const gchar *text,
		 guint len)
{
	char *escaped_text;

	if (text == NULL || len == 0) {
		vfs->path = g_strdup ("/");
		return;
	}

	if ( text[0] == '/' && text[1] == '/') {
		vfs->path = split_toplevel_uri (text + 2, len - 2,
						&vfs->host_name,
						&vfs->user_name,
						&vfs->host_port,
						&vfs->password);
	} else {
		vfs->path = g_strndup (text, len);
	}

	/* FIXME: this should be handled/supported by the specific method.
	 * This is a quick and dirty hack to minimize the amount of changes
	 * right before a milestone release.
	 *
	 * Do some method specific escaping. This for instance converts
	 * '?' to %3F in every method except "http" where it has a special
	 * meaning.
	 */
	if ( ! (strcmp (vfs->scheme, "http") == 0
	        || strcmp (vfs->scheme, "https") == 0
		|| strcmp (vfs->scheme, "dav") == 0
		|| strcmp (vfs->scheme, "davs") == 0
	        || strcmp (vfs->scheme, "help") == 0
	        || strcmp (vfs->scheme, "mate-help") == 0
	        || strcmp (vfs->scheme, "help") == 0
		)) {

		escaped_text = mate_vfs_escape_set (vfs->path, ";?&=+$,");
		g_free (vfs->path);
		vfs->path = escaped_text;
	}

	mate_vfs_remove_optional_escapes (vfs->path);
	mate_vfs_canonicalize_pathname (vfs->path);
}

/*
   split_toplevel_uri

   Extract hostname and username from "path" with length "path_len"

   examples:
       sunsite.unc.edu/pub/linux
       miguel@sphinx.nuclecu.unam.mx/c/nc
       tsx-11.mit.edu:8192/
       joe@foo.edu:11321/private
       joe:password@foo.se

   This function implements the following regexp: (whitespace for clarity)

   ( ( ([^:@/]*) (:[^@/]*)? @ )? ([^/:]*) (:([0-9]*)?) )?  (/.*)?
   ( ( ( user  ) (  pw  )?   )?   (host)    (port)?   )? (path <return value>)?

  It returns NULL if neither <host> nor <path> could be matched.

  port is checked to ensure that it does not exceed 0xffff.

  return value is <path> or is "/" if the path portion is not present
  All other arguments are set to 0 or NULL if their portions are not present

  pedantic: this function ends up doing an unbounded lookahead, making it
  potentially O(n^2) instead of O(n).  This could be avoided.  Realistically, though,
  its just the password field.

  Differences between the old and the new implemention:

                     Old                     New
  localhost:8080     host="localhost:8080"   host="localhost" port=8080
  /Users/mikef       host=""                 host=NULL

*/


#define URI_MOVE_PAST_DELIMITER \
	do {							\
		cur_tok_start = (++cur);			\
		if (path_end == cur) {				\
			success = FALSE;			\
			goto done;				\
		}						\
	} while (0);


#define uri_strlen_to(from, to)  ( (to) - (from) )
#define uri_strdup_to(from, to)  g_strndup ((from), uri_strlen_to((from), (to)))

typedef struct {
	const char *chrs;
	gboolean primed;
	char bv[32];
} UriStrspnSet;

static UriStrspnSet uri_strspn_sets[] = {
	{":@]" MATE_VFS_URI_PATH_STR, FALSE, ""},
	{"@" MATE_VFS_URI_PATH_STR, FALSE, ""},
	{":" MATE_VFS_URI_PATH_STR, FALSE, ""},
	{"]" MATE_VFS_URI_PATH_STR, FALSE, ""}
};

#define URI_DELIMITER_ALL_SET (uri_strspn_sets + 0)
#define URI_DELIMITER_USER_SET (uri_strspn_sets + 1)
#define URI_DELIMITER_HOST_SET (uri_strspn_sets + 2)
#define URI_DELIMITER_IPV6_SET (uri_strspn_sets + 3)

#define BV_SET(bv, idx) (bv)[((guchar)(idx))>>3] |= (1 << ( (idx) & 7) )
#define BV_IS_SET(bv, idx) ((bv)[((guchar)(idx))>>3] & (1 << ( (idx) & 7)))

static const char *
uri_strspn_to(const char *str, UriStrspnSet *set, const char *path_end)
{
	const char *cur;
	const char *cur_chr;

	if (!set->primed) {
		memset (set->bv, 0, sizeof(set->bv));

		for (cur_chr = set->chrs; '\0' != *cur_chr; cur_chr++) {
			BV_SET (set->bv, *cur_chr);
		}

		BV_SET (set->bv, '\0');
		set->primed = TRUE;
	}

	for (cur = str; cur < path_end && ! BV_IS_SET (set->bv, *cur); cur++)
		;

	if (cur >= path_end || '\0' == *cur) {
		return NULL;
	}

	return cur;
}

static gchar *
split_toplevel_uri (const gchar *path, guint path_len,
		    gchar **host_return, gchar **user_return,
		    guint *port_return, gchar **password_return)
{
	const char *path_end;
	const char *cur_tok_start;
	const char *cur;
	const char *next_delimiter;
	char *ret;
	char *host;
	gboolean success;

	g_assert (host_return != NULL);
	g_assert (user_return != NULL);
	g_assert (port_return != NULL);
	g_assert (password_return != NULL);

	*host_return = NULL;
	*user_return = NULL;
	*port_return = 0;
	*password_return = NULL;
	ret = NULL;

	success = FALSE;

	if (path == NULL || path_len == 0) {
		return g_strdup ("/");
	}


	path_end = path + path_len;

	cur_tok_start = path;
	cur = uri_strspn_to (cur_tok_start, URI_DELIMITER_ALL_SET, path_end);

	if (cur != NULL) {
		const char *tmp;

		if (*cur == ':') {
			/* This ':' belongs to username or IPv6 address.*/
			tmp = uri_strspn_to (cur_tok_start, URI_DELIMITER_USER_SET, path_end);

			if (tmp == NULL || *tmp != '@') {
				tmp = uri_strspn_to (cur_tok_start, URI_DELIMITER_IPV6_SET, path_end);

				if (tmp != NULL && *tmp == ']') {
					cur = tmp;
				}
			}
		}
	}

	if (cur != NULL) {

		/* Check for IPv6 address. */
		if (*cur == ']') {

			/*  No username:password in the URI  */
			/*  cur points to ']'  */

			cur = uri_strspn_to (cur, URI_DELIMITER_HOST_SET, path_end);
		}
	}

	if (cur != NULL) {
		next_delimiter = uri_strspn_to (cur, URI_DELIMITER_USER_SET, path_end);
	} else {
		next_delimiter = NULL;
	}

	if (cur != NULL
		&& (*cur == '@'
		    || (next_delimiter != NULL && *next_delimiter != '/' ))) {

		/* *cur == ':' or '@' and string contains a @ before a / */

		if (uri_strlen_to (cur_tok_start, cur) > 0) {
			char *tmp;
			tmp = uri_strdup_to (cur_tok_start,cur);
			*user_return = mate_vfs_unescape_string (tmp, NULL);
			g_free (tmp);
		}

		if (*cur == ':') {
			URI_MOVE_PAST_DELIMITER;

			cur = uri_strspn_to(cur_tok_start, URI_DELIMITER_USER_SET, path_end);

			if (cur == NULL || *cur != '@') {
				success = FALSE;
				goto done;
			} else if (uri_strlen_to (cur_tok_start, cur) > 0) {
				char *tmp;
				tmp = uri_strdup_to (cur_tok_start,cur);
				*password_return = mate_vfs_unescape_string (tmp, NULL);
				g_free (tmp);
			}
		}

		if (*cur != '/') {
			URI_MOVE_PAST_DELIMITER;

			/* Move cur to point to ':' after ']' */
			cur = uri_strspn_to (cur_tok_start, URI_DELIMITER_IPV6_SET, path_end);

			if (cur != NULL && *cur == ']') {  /* For IPv6 address */
				cur = uri_strspn_to (cur, URI_DELIMITER_HOST_SET, path_end);
			} else {
				cur = uri_strspn_to (cur_tok_start, URI_DELIMITER_HOST_SET, path_end);
			}
		} else {
			cur_tok_start = cur;
		}
	}

	if (cur == NULL) {
		/* [^:/]+$ */
		if (uri_strlen_to (cur_tok_start, path_end) > 0) {
			*host_return = uri_strdup_to (cur_tok_start, path_end);
			if (*(path_end - 1) == MATE_VFS_URI_PATH_CHR) {
				ret = g_strdup (MATE_VFS_URI_PATH_STR);
			} else {
				ret = g_strdup ("");
			}
			success = TRUE;
		} else { /* No host, no path */
			success = FALSE;
		}

		goto done;

	} else if (*cur == ':') {
		guint port;
		/* [^:/]*:.* */

		if (uri_strlen_to (cur_tok_start, cur) > 0) {
			*host_return = uri_strdup_to (cur_tok_start, cur);
		} else {
			success = FALSE;
			goto done;	/*No host but a port?*/
		}

		URI_MOVE_PAST_DELIMITER;

		port = 0;

		for ( ; cur < path_end && g_ascii_isdigit (*cur); cur++) {
			port *= 10;
			port += *cur - '0';
		}

		/* We let :(/.*)$ be treated gracefully */
		if (*cur != '\0' && *cur != MATE_VFS_URI_PATH_CHR) {
			success = FALSE;
			goto done;	/* ...but this would be an error */
		}

		if (port > 0xffff) {
			success = FALSE;
			goto done;
		}

		*port_return = port;

		cur_tok_start = cur;

	} else /* MATE_VFS_URI_PATH_CHR == *cur */ {
		/* ^[^:@/]+/.*$ */

		if (uri_strlen_to (cur_tok_start, cur) > 0) {
			*host_return = uri_strdup_to (cur_tok_start, cur);
		}

		cur_tok_start = cur;
	}

	if (*cur_tok_start != '\0' && uri_strlen_to (cur_tok_start, path_end) > 0) {
		ret = uri_strdup_to(cur, path_end);
	} else if (*host_return != NULL) {
		ret = g_strdup (MATE_VFS_URI_PATH_STR);
	}

	success = TRUE;

done:
	if (*host_return != NULL) {

		/* Check for an IPv6 address in square brackets.*/
		if (strchr (*host_return, '[') && strchr (*host_return, ']') && strchr (*host_return, ':')) {

			/* Extract the IPv6 address from square braced string. */
			host = g_ascii_strdown ((*host_return) + 1, strlen (*host_return) - 2);
		} else {
			host = g_ascii_strdown (*host_return, -1);
		}

		g_free (*host_return);
		*host_return = host;

	}

	/* If we didn't complete our mission, discard all the partials */
	if (!success) {
		g_free (*host_return);
		g_free (*user_return);
		g_free (*password_return);
		g_free (ret);

		*host_return = NULL;
		*user_return = NULL;
		*port_return = 0;
		*password_return = NULL;
		ret = NULL;
	}

	return ret;
}
