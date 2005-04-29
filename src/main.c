/*
 *  Leafpad - GTK+ based simple text editor
 *  Copyright (C) 2004-2005 Tarot Osuji
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

#define GLOBAL_VARIABLE_DEFINE
#include "leafpad.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

typedef struct {
	gint width;
	gint height;
	gchar *fontname;
	gboolean wordwrap;
	gboolean linenumbers;
	gboolean autoindent;
} Conf;

static void load_config_file(Conf *conf)
{
	FILE *fp;
	gchar *path;
	gchar buf[BUFSIZ];
	gchar **num;
	
	path = g_build_filename(g_get_home_dir(), "." PACKAGE, NULL);
	fp = fopen(path, "r");
	g_free(path);
	if (!fp)
		return;
	
	fgets(buf, sizeof(buf), fp);
	num = g_strsplit(buf, "." , 3);
	if ((atoi(num[1]) >= 8) && (atoi(num[2]) >= 0)) {
		fgets(buf, sizeof(buf), fp);
		conf->width = atoi(buf);
		fgets(buf, sizeof(buf), fp);
		conf->height = atoi(buf);
		fgets(buf, sizeof(buf), fp);
		g_free(conf->fontname);
		conf->fontname = g_strdup(buf);
		fgets(buf, sizeof(buf), fp);
		conf->wordwrap = atoi(buf);
		fgets(buf, sizeof(buf), fp);
		conf->linenumbers = atoi(buf);
		fgets(buf, sizeof(buf), fp);
		conf->autoindent = atoi(buf);
	}
	g_strfreev(num);
	fclose(fp);
}

void save_config_file(void)
{
	FILE *fp;
	gchar *path;
	GtkItemFactory *ifactory;
	gint width, height;
	gchar *fontname;
	gboolean wordwrap, linenumbers, autoindent;
	
	gtk_window_get_size(GTK_WINDOW(pub->mw->window), &width, &height);
	fontname = get_font_name_from_widget(pub->mw->view);
	ifactory = gtk_item_factory_from_widget(pub->mw->menubar);
	wordwrap = gtk_check_menu_item_get_active(
		GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item(ifactory,
			"/Options/Word Wrap")));
	linenumbers = gtk_check_menu_item_get_active(
		GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item(ifactory,
			"/Options/Line Numbers")));
	autoindent = gtk_check_menu_item_get_active(
		GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item(ifactory,
			"/Options/Auto Indent")));
	
	path = g_build_filename(g_get_home_dir(), "." PACKAGE, NULL);
	fp = fopen(path, "w");
	if (!fp) {
		g_print("%s: can't save config file - %s\n", PACKAGE, path);
		return;
	}
	g_free(path);
	
	fprintf(fp, "%s\n", PACKAGE_VERSION);
	fprintf(fp, "%d\n", width);
	fprintf(fp, "%d\n", height);
	fprintf(fp, "%s\n", fontname);
	fprintf(fp, "%d\n", wordwrap);
	fprintf(fp, "%d\n", linenumbers);
	fprintf(fp, "%d\n", autoindent);
	fclose(fp);
	
	g_free(fontname);
}

static struct option longopts[] = {
	{ "help", no_argument, 0, '?' },
	{ "codeset", required_argument, 0, 0 },
//	{ "charset", required_argument, 0, 0 },
//	{ "encoding", required_argument, 0, 0 },
	{ "version", no_argument, 0, 'v' },
	{ 0, 0, 0, 0 }
};

static void print_usage(void)
{
	g_print("Usage:\n");
	g_print("  %s \[OPTION...] \[filename]\n", PACKAGE);
	g_print("\n");
	g_print("Options:\n");
	g_print("  --codeset=CODESET             Set codeset to open file\n");
	g_print("  --display=DISPLAY             X display to use\n");
	g_print("  --screen=SCREEN               X screen to use\n");
	g_print("  --sync                        Make X calls synchronous\n");
	g_print("  --version                     Show version number\n");
	g_print("  --help                        Show this help\n");
}

static void parse_args(gint argc, gchar **argv, FileInfo *fi)
{
	EncArray *encarray;
	gint c, i;
	GError *error = NULL;
	
//	opterr = 0;
	do {
		c = getopt_long(argc, argv, "", longopts, NULL);
		switch (c) {
		case 0:
			if (optarg) {
				g_convert("TEST", -1, "UTF-8", optarg, NULL, NULL, &error);
				if (error) {
					g_error_free(error);
					error = NULL;
				} else {
					g_free(fi->charset);
					fi->charset = g_strdup(optarg);
				}
			}
			break;
		case 'v':
			g_print("%s\n", PACKAGE_STRING);
			exit(1);
		case '?':
			print_usage();
			exit(1);
		}
	} while (c != -1);
	
	if (fi->charset 
		&& (g_strcasecmp(fi->charset, get_default_charset()) != 0)
		&& (g_strcasecmp(fi->charset, "UTF-8") != 0)) {
		encarray = get_encoding_items(get_encoding_code());
		for (i = 0; i < ENCODING_MAX_ITEM_NUM; i++)
			if (encarray->item[i])
				if (g_strcasecmp(fi->charset, encarray->item[i]) == 0)
					break;
		if (i == ENCODING_MAX_ITEM_NUM)
			fi->charset_flag = TRUE;
	}
	
	if (optind < argc)
		fi->filename = parse_file_uri(argv[optind]);
}

gint main(gint argc, gchar **argv)
{
	Conf *conf;
	GtkItemFactory *ifactory;
	gchar *stdin_data = NULL;
	
	bindtextdomain(PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(PACKAGE, "UTF-8");
	textdomain(PACKAGE);
	
	gtk_init(&argc, &argv);
	
#if !GTK_CHECK_VERSION(2, 6, 0)
	add_about_stock();
#endif
	pub = g_malloc(sizeof(PublicData));
	pub->fi = g_malloc(sizeof(FileInfo));
	pub->fi->filename     = NULL;
	pub->fi->charset      = NULL;
	pub->fi->charset_flag = FALSE;
	pub->fi->lineend      = LF;
	
	parse_args(argc, argv, pub->fi);
	
	pub->mw = create_main_window();
	
	conf = g_malloc(sizeof(Conf));
	conf->width       = 600;
	conf->height      = 400;
	conf->fontname    = g_strdup("Monospace 12");
	conf->wordwrap    = FALSE;
	conf->linenumbers = FALSE;
	conf->autoindent  = FALSE;
	
	load_config_file(conf);
	
	gtk_window_set_default_size(
		GTK_WINDOW(pub->mw->window), conf->width, conf->height);
	set_text_font_by_name(pub->mw->view, conf->fontname);
	
	ifactory = gtk_item_factory_from_widget(pub->mw->menubar);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(
		gtk_item_factory_get_widget(ifactory, "/Options/Word Wrap")),
		conf->wordwrap);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(
		gtk_item_factory_get_widget(ifactory, "/Options/Line Numbers")),
		conf->linenumbers);
	indent_refresh_tab_width(pub->mw->view);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(
		gtk_item_factory_get_widget(ifactory, "/Options/Auto Indent")),
		conf->autoindent);
	
	gtk_widget_show_all(pub->mw->window);
	g_free(conf->fontname);
	g_free(conf);
	
	undo_init(pub->mw->view,
		gtk_item_factory_get_widget(ifactory, "/Edit/Undo"),
		gtk_item_factory_get_widget(ifactory, "/Edit/Redo"));
	dnd_init(pub->mw->view);
	
	if (pub->fi->filename)
		file_open_real(pub->mw->view, pub->fi);
	else
		stdin_data = gedit_utils_get_stdin();
	if (stdin_data) {
		gchar *str;
		GtkTextIter iter;
		
		str = g_convert(stdin_data, -1, "UTF-8",
			get_default_charset(), NULL, NULL, NULL);
		g_free(stdin_data);
	
//		gtk_text_buffer_set_text(buffer, "", 0);
		gtk_text_buffer_get_start_iter(pub->mw->buffer, &iter);
		gtk_text_buffer_insert(pub->mw->buffer, &iter, str, strlen(str));
		gtk_text_buffer_get_start_iter(pub->mw->buffer, &iter);
		gtk_text_buffer_place_cursor(pub->mw->buffer, &iter);
		gtk_text_buffer_set_modified(pub->mw->buffer, FALSE);
		gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(pub->mw->view), &iter, 0, FALSE, 0, 0);
		g_free(str);
	}
	
	set_main_window_title();
	
	gtk_main();
	
	return 0;
}
