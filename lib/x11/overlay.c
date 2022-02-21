/*****************************************************************
 * gmerlin - a general purpose multimedia framework and applications
 *
 * Copyright (c) 2001 - 2012 Members of the Gmerlin project
 * gmerlin-general@lists.sourceforge.net
 * http://gmerlin.sourceforge.net
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * *****************************************************************/

#include <string.h>
#include <x11/x11.h>
#include <x11/x11_window_private.h>

static gavl_video_frame_t * get_frame(void * priv)
  {
  overlay_stream_t * str = priv;
  if(!str->ovl)
    {
    str->ovl =
      str->win->current_driver->driver->create_overlay(str->win->current_driver, str);
    }

  if(str->win->current_driver->driver->unset_overlay)
    str->win->current_driver->driver->unset_overlay(str->win->current_driver, str);

  return str->ovl;
  }

static gavl_sink_status_t put_frame(void * priv, gavl_video_frame_t * frame)
  {
  bg_x11_window_t * w;
  overlay_stream_t * str = priv;
  
  w = str->win;

  if(frame && frame->src_rect.w && frame->src_rect.h)
    str->active = 1;
  else
    str->active = 0;

  if(str->win->current_driver->driver->set_overlay)
    str->win->current_driver->driver->set_overlay(str->win->current_driver, str);
  
  SET_FLAG(w, FLAG_OVERLAY_CHANGED);
  return GAVL_SINK_OK;
  }

gavl_video_sink_t *
bg_x11_window_add_overlay_stream(bg_x11_window_t * w,
                                 gavl_video_format_t * format)
  {
  overlay_stream_t * str;
  
  if(!w->current_driver->driver->init_overlay_stream)
    return NULL;
  
  w->overlay_streams =
    realloc(w->overlay_streams,
            (w->num_overlay_streams+1) * sizeof(*(w->overlay_streams)));

  w->overlay_streams[w->num_overlay_streams] =
    calloc(1, sizeof(*(w->overlay_streams[w->num_overlay_streams])));
  str = w->overlay_streams[w->num_overlay_streams];
  
  memset(str, 0, sizeof(*str));
  
  /* Initialize */
  gavl_video_format_copy(&str->format, format); 

  str->format.pixelformat = gavl_pixelformat_get_best(str->format.pixelformat, 
                                                      w->current_driver->ovl_formats, 0);
  
  w->current_driver->driver->init_overlay_stream(w->current_driver, str);
  
  gavl_video_format_copy(format, &str->format); 
  str->win = w;
  
  w->num_overlay_streams++;

  str->sink = gavl_video_sink_create(w->current_driver->driver->create_overlay ?
                                     get_frame : NULL,
                                     put_frame, str, &str->format);
  
  return str->sink;
  }

