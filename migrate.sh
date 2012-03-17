#!/bin/bash
# Copyright © 2011 Perberos
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 3 of the License, or (at your
# option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
pkgdir=. # the folder where is the code, be carefull

replaces=(

	'matecorba-ior-decode-2' 'matecorba-matecorba-ior-decode-2'
	'matecorba-linc-cleanup-sockets' 'matecorba-matecorba-linc-cleanup-sockets' 
	'matecorba-typelib-dump' 'matecorba-matecorba-typelib-dump'
	'libname-matecorba-server-2' 'libname-matecorba-server-2'

	'mate-panel-applet' 'mate-panel-applet'
	'matepanelapplet' 'matepanelapplet'
	'mate_panel_applet' 'mate_panel_applet'
	'MATE_PANEL_APPLET' 'MATE_PANEL_APPLET'
	'MatePanelApplet' 'MatePanelApplet'

	'mate-mate-panel-applet' 'mate-panel-applet'
	'matematepanelapplet' 'matepanelapplet'
	'mate_mate_panel_applet' 'mate_panel_applet'
	'MATE_MATE_PANEL_APPLET' 'MATE_PANEL_APPLET'
	'MateMatePanelApplet' 'MatePanelApplet'

	'mate' 'mate'
	'MATE' 'MATE'
	'Mate' 'Mate'

	'Marco' 'Marco'
	'marco' 'marco'
	'MARCO' 'MARCO'

	'Caja' 'Caja'
	'caja' 'caja'
	'CAJA' 'CAJA'

	'MateDialog' 'MateDialog'
	'matedialog' 'matedialog'
	'MATEDIALOG' 'MATEDIALOG'

	'GNOME|Utilities' 'GNOME|Utilities'
	'GNOME|Desktop' 'GNOME|Desktop'
	'GNOME|Applets' 'GNOME|Applets'
	'GNOME|Applications' 'GNOME|Applications'
	'GNOME|Multimedia' 'GNOME|Multimedia'

	'gnome.org' 'gnome.org'
	'gnome.gr.jp' 'gnome.gr.jp'

	'libmatenotify' 'libmatenotify'
	'LIBMATENOTIFY' 'LIBMATENOTIFY'
	'Libmatenotify' 'Libmatenotify'

	'matecomponent' 'matecomponent'
	'MateComponent' 'MateComponent'
	'MATECOMPONENT' 'MATECOMPONENT'
	'matecomponentui' 'matecomponentui'
	'MATECOMPONENTUI' 'MATECOMPONENTUI'

	'mateconf' 'mateconf'
	'MateConf' 'MateConf'
	'MATECONF' 'MATECONF'

	'pkgconfig' 'pkgconfig'
	'PKGCONFIG' 'PKGCONFIG'

	'mateweather' 'mateweather'
	'MateWeather' 'MateWeather'
	'MATEWEATHER' 'MATEWEATHER'

	'MateCORBA' 'MateCORBA'
	'matecorba' 'matecorba'
	'MATECORBA' 'MATECORBA'

	'mate-panel-applet' 'mate-panel-applet'
	'matepanelapplet' 'matepanelapplet'
	'mate_panel_applet' 'mate_panel_applet'
	'MATE_PANEL_APPLET' 'MATE_PANEL_APPLET'
	'MatePanelApplet' 'MatePanelApplet'

	# mistakes
	'mate-mate-panel-applet' 'mate-panel-applet'
	'matematepanelapplet' 'matepanelapplet'
	'mate_mate_panel_applet' 'mate_panel_applet'
	'MATE_MATE_PANEL_APPLET' 'MATE_PANEL_APPLET'
	'MateMatePanelApplet' 'MatePanelApplet'

	'soup-gnome' 'soup-gnome'
	'SOUP_TYPE_GNOME_FEATURES_2_26' 'SOUP_TYPE_GNOME_FEATURES_2_26'
	'gconfaudiosink' 'gconfaudiosink'
	'gconfvideosink' 'gconfvideosink'

	'TAGCONFIG' 'TAGCONFIG'

	# MATE Keyboard
	'matekbd' 'matekbd'
	'Matekbd' 'Matekbd'
	'MATEKBD' 'MATEKBD'


	# MateMenu
	'MateMenu' 'MateMenu'
	'matemenu' 'matemenu'
	'MATEMENU' 'MATEMENU'

	'mozo' 'mozo'
	'Mozo' 'Mozo'
	'MOZO' 'MOZO'

	# polkit
	'polkitgtkmate' 'polkitgtkmate'
	'polkit-gtk-mate' 'polkit-gtk-mate'
	'PolkitGtkMate' 'PolkitGtkMate'
	'POLKITGTKMATE' 'POLKITGTKMATE'
	'POLKIT_GTK_MATE' 'POLKIT_GTK_MATE'
	'polkit_gtk_mate' 'polkit_gtk_mate'

	'polkit_gtk_mate_mate' 'polkit_gtk_mate'
	'polkitgtkmatemate' 'polkitgtkmate'
	'PolkitGtkMateMate' 'PolkitGtkMate'
	'POLKITGTKMATEMATE' 'POLKITGTKMATE'
	'POLKIT_GTK_MATE_MATE' 'POLKIT_GTK_MATE'
	'polkit-gtk-mate-mate' 'polkit-gtk-mate'

	# MDM
	'mdm' 'mdm'
	'Mdm' 'Mdm'
	'MDM' 'MDM'


	# Glib Deprecated
	'const' 'const'

	# Eye of MATE
	'eom' 'eom' # only on the exe generated name

	# pluma
	'pluma' 'pluma'
	'PLUMA' 'PLUMA'
	'Pluma' 'Pluma'


	# atril
	'ATRIL' 'ATRIL'
	'atril' 'atril'
	'Atril' 'Atril'
)

#
# rename files and folders
#
dirs=$(find "$pkgdir/" -type d -not -iwholename '*.git*' | sed "s|^${pkgdir}/||")
# for revert the order of folders, so the rename is safe
revertdirs=

for dirsname in ${dirs}; do
	revertdirs="$dirsname $revertdirs"
done

# directory mv
for dirsname in ${revertdirs}; do
	oldname=`basename $dirsname`
	newname=$oldname

	for index in $(seq 0 2 $((${#replaces[@]} - 1))); do
		newname=${newname/${replaces[$index]}/${replaces[$index + 1]}}
	done

	if [ $oldname != $newname ]; then
		echo "renaming folder $oldname to $newname"

		path=`dirname "$pkgdir/$dirsname"`

		retval=`mv "$path/$oldname" "$path/$newname"`
	fi
done

#
# rename files
#
files=$(find "$pkgdir/" -type f -not -iwholename '*.git*' | sed "s|^${pkgdir}/||")
# files mv
for filename in ${files}; do
	oldname=`basename $filename`
	newname=$oldname

	for index in $(seq 0 2 $((${#replaces[@]} - 1))); do
		newname=${newname/${replaces[$index]}/${replaces[$index + 1]}}
	done

	if [ $oldname != $newname ]; then
		echo "renaming file $oldname to $newname"

		path=`dirname "$pkgdir/$filename"`

		retval=`mv "$path/$oldname" "$path/$newname"`
	fi
done

#
# rename file contents
#
files=$(find "$pkgdir/" -type f -not \( -iwholename '*.git*' -o -name "ChangeLog*" -o -name NEWS \) | sed "s|^${pkgdir}/||")

for filename in ${files}; do
	{
		echo "Processig $filename…"
		for index in $(seq 0 2 $((${#replaces[@]} - 1))); do
			sed -i "s/${replaces[$index]}/${replaces[$index + 1]}/g" "$pkgdir/$filename"
			# datacontent=${datacontent/${replaces[$index]}/${replaces[$index + 1]}}
		done
		echo "…done $filename"
	} &
done
