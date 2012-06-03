/*
 * Preferences parser
 *
 * Copyright (C) 2006-2009 Jorge Arellano Cid <jcid@dillo.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include <sys/types.h>
#include <stdlib.h>
#include <locale.h>            /* for setlocale */

#include "prefs.h"
#include "misc.h"
#include "msg.h"
#include "colors.h"

#include "prefsparser.hh"

typedef enum {
   PREFS_BOOL,
   PREFS_COLOR,
   PREFS_STRING,
   PREFS_STRINGS,
   PREFS_URL,
   PREFS_INT32,
   PREFS_DOUBLE,
   PREFS_GEOMETRY,
   PREFS_FILTER,
   PREFS_PANEL_SIZE
} PrefType_t;

typedef struct SymNode_ {
   const char *name;
   void *pref;
   PrefType_t type;
} SymNode_t;

/*
 * Parse a name/value pair and set preferences accordingly.
 */
int PrefsParser::parseOption(char *name, char *value)
{
   const SymNode_t *node;
   uint_t i;
   int st;

   /* Symbol array, sorted alphabetically */
   const SymNode_t symbols[] = {
      { "allow_white_bg", &prefs.allow_white_bg, PREFS_BOOL },
      { "bg_color", &prefs.bg_color, PREFS_COLOR },
      { "buffered_drawing", &prefs.buffered_drawing, PREFS_INT32 },
      { "contrast_visited_color", &prefs.contrast_visited_color, PREFS_BOOL },
      { "enterpress_forces_submit", &prefs.enterpress_forces_submit,
        PREFS_BOOL },
      { "filter_auto_requests", &prefs.filter_auto_requests, PREFS_FILTER },
      { "focus_new_tab", &prefs.focus_new_tab, PREFS_BOOL },
      { "font_cursive", &prefs.font_cursive, PREFS_STRING },
      { "font_factor", &prefs.font_factor, PREFS_DOUBLE },
      { "font_fantasy", &prefs.font_fantasy, PREFS_STRING },
      { "font_max_size", &prefs.font_max_size, PREFS_INT32 },
      { "font_min_size", &prefs.font_min_size, PREFS_INT32 },
      { "font_monospace", &prefs.font_monospace, PREFS_STRING },
      { "font_sans_serif", &prefs.font_sans_serif, PREFS_STRING },
      { "font_serif", &prefs.font_serif, PREFS_STRING },
      { "fullwindow_start", &prefs.fullwindow_start, PREFS_BOOL },
      { "geometry", NULL, PREFS_GEOMETRY },
      { "home", &prefs.home, PREFS_URL },
      { "http_language", &prefs.http_language, PREFS_STRING },
      { "http_max_conns", &prefs.http_max_conns, PREFS_INT32 },
      { "http_proxy", &prefs.http_proxy, PREFS_URL },
      { "http_proxyuser", &prefs.http_proxyuser, PREFS_STRING },
      { "http_referer", &prefs.http_referer, PREFS_STRING },
      { "http_user_agent", &prefs.http_user_agent, PREFS_STRING },
      { "limit_text_width", &prefs.limit_text_width, PREFS_BOOL },
      { "load_images", &prefs.load_images, PREFS_BOOL },
      { "load_stylesheets", &prefs.load_stylesheets, PREFS_BOOL },
      { "middle_click_drags_page", &prefs.middle_click_drags_page,
        PREFS_BOOL },
      { "middle_click_opens_new_tab", &prefs.middle_click_opens_new_tab,
        PREFS_BOOL },
      { "right_click_closes_tab", &prefs.right_click_closes_tab, PREFS_BOOL },
      { "no_proxy", &prefs.no_proxy, PREFS_STRING },
      { "panel_size", &prefs.panel_size, PREFS_PANEL_SIZE },
      { "parse_embedded_css", &prefs.parse_embedded_css, PREFS_BOOL },
      { "save_dir", &prefs.save_dir, PREFS_STRING },
      { "search_url", &prefs.search_urls, PREFS_STRINGS },
      { "show_back", &prefs.show_back, PREFS_BOOL },
      { "show_bookmarks", &prefs.show_bookmarks, PREFS_BOOL },
      { "show_clear_url", &prefs.show_clear_url, PREFS_BOOL },
      { "show_extra_warnings", &prefs.show_extra_warnings, PREFS_BOOL },
      { "show_filemenu", &prefs.show_filemenu, PREFS_BOOL },
      { "show_forw", &prefs.show_forw, PREFS_BOOL },
      { "show_help", &prefs.show_help, PREFS_BOOL },
      { "show_home", &prefs.show_home, PREFS_BOOL },
      { "show_msg", &prefs.show_msg, PREFS_BOOL },
      { "show_progress_box", &prefs.show_progress_box, PREFS_BOOL },
      { "show_quit_dialog", &prefs.show_quit_dialog, PREFS_BOOL },
      { "show_reload", &prefs.show_reload, PREFS_BOOL },
      { "show_save", &prefs.show_save, PREFS_BOOL },
      { "show_search", &prefs.show_search, PREFS_BOOL },
      { "show_stop", &prefs.show_stop, PREFS_BOOL },
      { "show_tools", &prefs.show_tools, PREFS_BOOL },
      { "show_tooltip", &prefs.show_tooltip, PREFS_BOOL },
      { "show_url", &prefs.show_url, PREFS_BOOL },
      { "small_icons", &prefs.small_icons, PREFS_BOOL },
      { "start_page", &prefs.start_page, PREFS_URL },
      { "theme", &prefs.theme, PREFS_STRING },
      { "w3c_plus_heuristics", &prefs.w3c_plus_heuristics, PREFS_BOOL }
   };

   node = NULL;
   for (i = 0; i < sizeof(symbols) / sizeof(SymNode_t); i++) {
      if (!strcmp(symbols[i].name, name)) {
         node = & (symbols[i]);
         break;
      }
   }

   if (!node) {
      MSG("prefs: {%s} is not a recognized token.\n", name);
      return -1;
   }

   switch (node->type) {
   case PREFS_BOOL:
      *(bool_t *)node->pref = (!dStrAsciiCasecmp(value, "yes") ||
                               !dStrAsciiCasecmp(value, "true"));
      break;
   case PREFS_COLOR:
      *(int32_t *)node->pref = a_Color_parse(value, *(int32_t*)node->pref,&st);
      if (st)
         MSG("prefs: Color '%s' not recognized.\n", value);
      break;
   case PREFS_STRING:
      dFree(*(char **)node->pref);
      *(char **)node->pref = dStrdup(value);
      break;
   case PREFS_STRINGS:
   {
      Dlist *lp = *(Dlist **)node->pref;
      if (dList_length(lp) == 2 && !dList_nth_data(lp, 1)) {
         /* override the default */
         void *data = dList_nth_data(lp, 0);
         dList_remove(lp, data);
         dList_remove(lp, NULL);
         dFree(data);
      }
      dList_append(lp, dStrdup(value));
      break;
   }
   case PREFS_URL:
      a_Url_free(*(DilloUrl **)node->pref);
      *(DilloUrl **)node->pref = a_Url_new(value, NULL);
      break;
   case PREFS_INT32:
      *(int32_t *)node->pref = strtol(value, NULL, 10);
      break;
   case PREFS_DOUBLE:
      *(double *)node->pref = strtod(value, NULL);
      break;
   case PREFS_GEOMETRY:
      a_Misc_parse_geometry(value, &prefs.xpos, &prefs.ypos,
                            &prefs.width, &prefs.height);
      break;
   case PREFS_FILTER:
      if (!dStrAsciiCasecmp(value, "same_domain"))
         prefs.filter_auto_requests = PREFS_FILTER_SAME_DOMAIN;
      else {
         if (dStrAsciiCasecmp(value, "allow_all"))
            MSG_WARN("prefs: unrecognized value for filter_auto_requests\n");
         prefs.filter_auto_requests = PREFS_FILTER_ALLOW_ALL;
      }
      break;
   case PREFS_PANEL_SIZE:
      if (!dStrAsciiCasecmp(value, "tiny"))
         prefs.panel_size = P_tiny;
      else if (!dStrAsciiCasecmp(value, "small"))
         prefs.panel_size = P_small;
      else /* default to "medium" */
         prefs.panel_size = P_medium;
      break;
   default:
      MSG_WARN("prefs: {%s} IS recognized but not handled!\n", name);
      break;   /* Not reached */
   }
   return 0;
}

/*
 * Parses the dillorc and sets the values in the prefs structure.
 */
void PrefsParser::parse(FILE *fp)
{
   char *line, *name, *value, *oldLocale;
   int st;

   // changing the LC_NUMERIC locale (temporarily) to C
   // avoids parsing problems with float numbers
   oldLocale = dStrdup(setlocale(LC_NUMERIC, NULL));
   setlocale(LC_NUMERIC, "C");

   // scan the file line by line
   while ((line = dGetline(fp)) != NULL) {
      st = dParser_parse_rc_line(&line, &name, &value);

      if (st == 0) {
         _MSG("prefsparser: name=%s, value=%s\n", name, value);
         parseOption(name, value);
      } else if (st < 0) {
         MSG_ERR("prefsparser: Syntax error in dillorc:"
                 " name=\"%s\" value=\"%s\"\n", name, value);
      }

      dFree(line);
   }
   fclose(fp);

   // restore the old numeric locale
   setlocale(LC_NUMERIC, oldLocale);
   dFree(oldLocale);
}
