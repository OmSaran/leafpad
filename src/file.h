/*
 *  file.h
 *  This file is part of Leafpad
 *
 *  Copyright (C) 2004 Tarot Osuji
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _FILE_H
#define _FILE_H

/* File Information */
typedef struct {
	gchar *filepath;
	gchar *charset;
	gint line_ending;
	gchar *manual_charset;
} FileInfo;

/* Dialog Mode */
enum {
	OPEN = 0,
	SAVE
};

gint file_open_real(GtkWidget *textview, FileInfo *fi);
gint file_save_real(GtkWidget *textview, FileInfo *fi);
//gchar *get_file_name_by_selector(GtkWidget *window, const gchar *title, gchar *default_filepath);
FileInfo *get_file_info_by_selector(GtkWidget *window, gint mode, FileInfo *fi);

#endif /* _FILE_H */