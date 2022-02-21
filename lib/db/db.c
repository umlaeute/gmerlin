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

#define _XOPEN_SOURCE
#define _GNU_SOURCE

#include <config.h>
#include <unistd.h>

#include <mediadb_private.h>
#include <gmerlin/log.h>
#include <gmerlin/utils.h>
#include <string.h>
#include <ctype.h>

#define LOG_DOMAIN "db"

static const char * create_commands[] =
  {

    "CREATE TABLE OBJECTS ("
    "ID INTEGER PRIMARY KEY, "
    "TYPE INTEGER, "
    "REF_ID INTEGER, "
    "PARENT_ID INTEGER, "
    "CHILDREN INTEGER, "
    "SIZE INTEGER, "
    "DURATION INTEGER, "
    "LABEL INTEGER);"
    
    "CREATE TABLE DIRECTORIES ("
    "ID INTEGER PRIMARY KEY, "
    "PATH TEXT, "
    "SCAN_FLAGS INTEGER, "
    "UPDATE_ID INTEGER, "
    "SCAN_DIR_ID INTEGER"
    ");",

    "CREATE TABLE FILES ("
    "ID INTEGER PRIMARY KEY, "
    "PATH TEXT, "
    "MTIME TEXT, "
    "MIMETYPE INTEGER, "
    "SCAN_DIR_ID INTEGER" 
    ");",

    /* String lookup tables */

    "CREATE TABLE MIMETYPES("
    "ID INTEGER PRIMARY KEY, "
    "NAME TEXT"
    ");",
    
    "CREATE TABLE AUDIO_GENRES("
    "ID INTEGER PRIMARY KEY, "
    "NAME TEXT"
    ");",

    "CREATE TABLE AUDIO_ARTISTS("
    "ID INTEGER PRIMARY KEY, "
    "NAME TEXT"
    ");",

    "CREATE TABLE VIDEO_PERSONS("
    "ID INTEGER PRIMARY KEY, "
    "NAME TEXT"
    ");",

    "CREATE TABLE VIDEO_GENRES("
    "ID INTEGER PRIMARY KEY, "
    "NAME TEXT"
    ");",

    "CREATE TABLE VIDEO_COUNTRIES("
    "ID INTEGER PRIMARY KEY, "
    "NAME TEXT"
    ");",

    "CREATE TABLE VIDEO_LANGUAGES("
    "ID INTEGER PRIMARY KEY, "
    "NAME TEXT"
    ");",

    "CREATE TABLE AUDIO_FILES("
    "ID INTEGER PRIMARY KEY, "
    "TITLE TEXT, "
    "SEARCH_TITLE TEXT, "
    "ARTIST INTEGER, "
    "GENRE INTEGER, "
    "DATE TEXT, "
    "ALBUM INTEGER, "
    "TRACK INTEGER, "
    "BITRATE TEXT, "
    "SAMPLERATE INTEGER, "
    "CHANNELS INTEGER"
    ");",

    "CREATE TABLE IMAGE_FILES("
    "ID INTEGER PRIMARY KEY, "
    "WIDTH INTEGER, "
    "HEIGHT INTEGER, "
    "DATE TEXT"
    ");"

    "CREATE TABLE AUDIO_ALBUMS("
    "ID INTEGER PRIMARY KEY, "
    "ARTIST INTEGER, "
    "TITLE TEXT, "
    "SEARCH_TITLE TEXT, "
    "GENRE INTEGER, "
    "COVER INTEGER, " 
    "DATE TEXT"
    ");",
   
    "CREATE TABLE PLAYLISTS("
    "ID INTEGER PRIMARY KEY, "
    "PLAYLIST_ID INTEGER, "
    "ITEM_ID INTEGER, "
    "IDX INTEGER"
    ");",

    "CREATE TABLE VIDEO_FILES("
    "ID INTEGER PRIMARY KEY, "
    "WIDTH INTEGER, "
    "HEIGHT INTEGER, "
    "ASPECT TEXT, "
    "TIMESCALE INTEGER, "
    "FRAME_DURATION INTEGER, "
    "VIDEO_CODEC TEXT, "
    "COLLECTION INTEGER, "
    "IDX INTEGER"
    ");",

    "CREATE TABLE VIDEO_ACTORS_VIDEOS("
    "ID INTEGER PRIMARY KEY, "
    "VIDEO_ID INTEGER, "
    "PERSON_ID INTEGER, "
    "UNIQUE(VIDEO_ID, PERSON_ID)"
    ");",

    "CREATE TABLE VIDEO_DIRECTORS_VIDEOS("
    "ID INTEGER PRIMARY KEY, "
    "VIDEO_ID INTEGER, "
    "PERSON_ID INTEGER, "
    "UNIQUE(VIDEO_ID, PERSON_ID)"
    ");",

    "CREATE TABLE VIDEO_GENRES_VIDEOS("
    "ID INTEGER PRIMARY KEY, "
    "VIDEO_ID INTEGER, "
    "GENRE_ID INTEGER, "
    "UNIQUE(VIDEO_ID, GENRE_ID)"
    ");",

    "CREATE TABLE VIDEO_COUNTRIES_VIDEOS("
    "ID INTEGER PRIMARY KEY, "
    "VIDEO_ID INTEGER, "
    "COUNTRY_ID INTEGER, "
    "UNIQUE(VIDEO_ID, COUNTRY_ID)"
    ");",

    "CREATE TABLE VIDEO_AUDIO_LANGUAGES_VIDEOS("
    "ID INTEGER PRIMARY KEY, "
    "VIDEO_ID INTEGER, "
    "LANGUAGE_ID INTEGER, "
    "UNIQUE(VIDEO_ID, LANGUAGE_ID)"
    ");",

    "CREATE TABLE VIDEO_SUBTITLE_LANGUAGES_VIDEOS("
    "ID INTEGER PRIMARY KEY, "
    "VIDEO_ID INTEGER, "
    "LANGUAGE_ID INTEGER, "
    "UNIQUE(VIDEO_ID, LANGUAGE_ID)"
    ");",

    "CREATE TABLE VIDEO_INFOS("
    "ID INTEGER PRIMARY KEY, "
    "TITLE TEXT, "
    "SEARCH_TITLE TEXT, "
    "TITLE_ORIG TEXT, "
    "RATING TEXT, "
    "PARENTAL_CONTROL TEXT, "
    "DATE TEXT, "
    "PLOT TEXT, "
    "POSTER INTEGER, "
    "FANART INTEGER, "
    "NFO_FILE INTEGER, "
    "VIDEO_TYPE INTEGER"
    ");",

    "CREATE TABLE URLS("
    "ID INTEGER PRIMARY KEY, "
    "STREAM_URL TEXT, "
    "STATION_URL TEXT, "
    "MIMETYPE INTEGER,"
    "AUDIO_BITRATE TEXT, "
    "SAMPLERATE INTEGER, "
    "CHANNELS INTEGER"
    ");",

    "CREATE TABLE ROOTFOLDERS("
    "ID INTEGER PRIMARY KEY, "
    "TYPE INTEGER, "
    "IX INTEGER"
    ");",
    
    "CREATE INDEX AUDIO_FILES_ALBUM ON AUDIO_FILES (ALBUM);",
    "CREATE INDEX OBJECTS_TYPE ON OBJECTS (TYPE);",
    "CREATE INDEX OBJECTS_PARENT_ID ON OBJECTS (PARENT_ID);",
    "CREATE INDEX FILES_PATH ON FILES (PATH);",
    "CREATE INDEX DIRECTORIES_PATH ON DIRECTORIES(PATH);",
    NULL,
  };

static void build_database(bg_db_t * db)
  {
  void * obj;
  int i = 0;
  while(create_commands[i])
    {
    if(!bg_sqlite_exec(db->db, create_commands[i], NULL, NULL))
      return;
    i++;
    }
  bg_db_create_tables_vfolders(db);

  /* Create root container */
  obj = bg_db_object_create_root(db);
  bg_db_object_set_type(obj, BG_DB_OBJECT_ROOT);
  bg_db_object_set_label(obj, "Root");
  bg_db_object_set_parent_id(db, obj, -1);
  bg_db_object_unref(obj);
  bg_db_flush(db);
  }

const char * directories[] =
  {
    "gmerlin-db",
    "gmerlin-db/previews",
    "gmerlin-db/thumbnails",
    NULL,
  };

bg_db_t * bg_db_create(const char * path,
                       bg_plugin_registry_t * plugin_reg, int create)
  {
  int result;
  int exists = 0;
  sqlite3 * db;
  bg_db_t * ret;
  int i;
  char * tmp_string;
  const char * sql;
  tmp_string = bg_sprintf("%s/gmerlin-db", path);
  
  if(!access(tmp_string, R_OK | W_OK))
    exists = 1;
  free(tmp_string);

  if(!exists && !create)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,
           "Cannot open database in %s (maybe use create option)", path);
    return NULL;
    }

  if(exists && create)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,
           "Database in %s already exists", path);
    return NULL;
    }

  if(create && !exists)
    {
    i = 0;
    while(directories[i])
      {
      tmp_string = bg_sprintf("%s/%s", path, directories[i]);
      bg_ensure_directory(tmp_string, 0);
      free(tmp_string);
      i++;
      }
    }
  
  /* Create database */

  tmp_string = bg_sprintf("%s/gmerlin-db/gmerlin.db", path);
  result = sqlite3_open(tmp_string, &db);

  if(result)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot open database %s: %s", tmp_string,
           sqlite3_errmsg(db));
    sqlite3_close(db);
    free(tmp_string);
    return NULL;
    }

  ret = calloc(1, sizeof(*ret));
  ret->db = db;
  ret->db_path = tmp_string;
  ret->plugin_reg = plugin_reg;

  ret->cache_size = 256;
  ret->cache = calloc(ret->cache_size, sizeof(*ret->cache));

  for(i = 0; i < ret->cache_size; i++)
    bg_db_object_init(&ret->cache[i]);
                      
  if(!exists)
    build_database(ret);
  
  /* Base path */
  ret->base_dir = bg_canonical_filename(path);
  ret->base_dir = gavl_strcat(ret->base_dir, "/");
  ret->base_len = strlen(ret->base_dir);

  /* String cache */
  ret->audio_artists =
    bg_db_string_cache_create(256,
                              "AUDIO_ARTISTS",
                              "NAME", "ID");

  ret->audio_genres =
    bg_db_string_cache_create(16,
                              "AUDIO_GENRES",
                              "NAME", "ID");

  ret->video_genres =
    bg_db_string_cache_create(16,
                              "VIDEO_GENRES",
                              "NAME", "ID");

  ret->video_persons =
    bg_db_string_cache_create(256,
                              "VIDEO_PERSONS",
                              "NAME", "ID");

  ret->video_countries =
    bg_db_string_cache_create(16,
                              "VIDEO_COUNTRIES",
                              "NAME", "ID");

  ret->video_languages =
    bg_db_string_cache_create(16,
                              "VIDEO_LANGUAGES",
                              "NAME", "ID");
 
  ret->audio_albums =
    bg_db_string_cache_create(256,
                              "AUDIO_ALBUMS",
                              "TITLE", "ID");

  ret->mimetypes =
    bg_db_string_cache_create(16,
                              "MIMETYPES",
                              "NAME", "ID");

  bg_db_thumbnail_cache_init(&ret->th_cache);
  
  /* Pre-compile select statements */

  sql = "SELECT * FROM OBJECTS WHERE ID = ?1;";
  if(sqlite3_prepare_v2(ret->db, sql, strlen(sql)+1, &ret->q_objects, NULL) != SQLITE_OK)
    return NULL;
  
  sql = "SELECT * FROM FILES WHERE ID = ?1;";
  if(sqlite3_prepare_v2(ret->db, sql, strlen(sql)+1, &ret->q_files, NULL) != SQLITE_OK)
    return NULL;
  
  sql = "SELECT * FROM AUDIO_FILES WHERE ID = ?1;";
  if(sqlite3_prepare_v2(ret->db, sql, strlen(sql)+1, &ret->q_audio_files, NULL) != SQLITE_OK)
    return NULL;

  sql = "SELECT * FROM VIDEO_FILES WHERE ID = ?1;";
  if(sqlite3_prepare_v2(ret->db, sql, strlen(sql)+1, &ret->q_video_files, NULL) != SQLITE_OK)
    return NULL;
  
  sql = "SELECT * FROM AUDIO_ALBUMS WHERE ID = ?1;";
  if(sqlite3_prepare_v2(ret->db, sql, strlen(sql)+1, &ret->q_audio_albums, NULL) != SQLITE_OK)
    return NULL;

  sql = "SELECT * FROM DIRECTORIES WHERE ID = ?1;";
  if(sqlite3_prepare_v2(ret->db, sql, strlen(sql)+1, &ret->q_directories, NULL) != SQLITE_OK)
    return NULL;

  sql = "SELECT * FROM VIDEO_INFOS WHERE ID = ?1;";
  if(sqlite3_prepare_v2(ret->db, sql, strlen(sql)+1, &ret->q_video_infos, NULL) != SQLITE_OK)
    return NULL;

  sql = "SELECT * FROM URLS WHERE ID = ?1;";
  if(sqlite3_prepare_v2(ret->db, sql, strlen(sql)+1, &ret->q_urls, NULL) != SQLITE_OK)
    return NULL;
  
  sql = "SELECT * FROM ROOTFOLDERS WHERE ID = ?1;";
  if(sqlite3_prepare_v2(ret->db, sql, strlen(sql)+1, &ret->q_rootfolders, NULL) != SQLITE_OK)
    return NULL;
  
  return ret;
  }

const char * bg_db_get_filename(bg_db_t * db)
  {
  return db->db_path;
  }


void bg_db_start_transaction(bg_db_t * db)
  {
  bg_sqlite_exec(db->db, "BEGIN TRANSACTION;", NULL, NULL);
  }

void bg_db_end_transaction(bg_db_t * db)
  {
  bg_sqlite_exec(db->db, "END;", NULL, NULL);
  }

char * bg_db_filename_to_abs(bg_db_t * db, char * filename)
  {
  char * ret;
  ret = bg_sprintf("%s%s", db->base_dir, filename);
  free(filename);
  return ret;
  }

const char * bg_db_filename_to_rel(bg_db_t * db, const char * filename)
  {
  return filename + db->base_len;
  }

// YYYY-MM-DD HH:MM:SS
#define TIME_FORMAT "%Y-%m-%d %H:%M:%S"

time_t bg_db_string_to_time(const char * str)
  {
  struct tm tm;
  memset(&tm, 0, sizeof(tm));
  strptime(str, TIME_FORMAT, &tm); 
  return timegm(&tm);
  }

char * bg_db_time_to_string(time_t time, char * str)
  {
  struct tm tm;
  memset(&tm, 0, sizeof(tm));
  gmtime_r(&time, &tm);
  strftime(str, BG_DB_TIME_STRING_LEN, TIME_FORMAT, &tm);
  return str;
  }

static int flush_object(bg_db_t * db, void * obj)
  {
  bg_db_obj_cache_t * st = obj;
  st->sync = 1;
  if(memcmp(&st->obj, &st->orig, sizeof(st->obj)))
    {
    //    fprintf(stderr, "Flush %ld\n", st->obj.obj.id);
    bg_db_object_update(db, &st->obj, 1, 1);
    memcpy(&st->orig, &st->obj, sizeof(st->obj));
    return 1;
    }
  return 0;
  }

static int children_in_cache(bg_db_t * db, int i)
  {
  int j;
  int64_t id = bg_db_object_get_id(&db->cache[i]);
  
  for(j = 0; j < db->cache_size; j++)
    {
    if(j != i)
      {
      if(!db->cache[j].sync &&
         (db->cache[j].obj.obj.parent_id == id))
        {
        // fprintf(stderr, "%"PRId64" is child of %"PRId64" in cache\n",
        //         db->cache[j].obj.obj.id, id);
        return 1;
        }
      }
    }
  return 0;
  }

void bg_db_obj_cache_clear(bg_db_obj_cache_t * ci)
  {
  bg_db_object_free(&ci->obj);
  bg_db_object_init(&ci->obj);
  bg_sqlite_id_tab_free(&ci->children);
  bg_sqlite_id_tab_init(&ci->children);
  ci->have_children = 0;
  ci->refcount      = 0;
  }


static void db_flush(bg_db_t * db, int do_free)
  {
  int i;
  int done = 0;
  int dead_refs = 0;
  
  /* Pass 1: Flush all leaf objects */
  for(i = 0; i < db->cache_size; i++)
    {
    if(!bg_db_object_is_valid(&db->cache[i]))
      {
      db->cache[i].sync = 1;
      continue;
      }
    if(!(bg_db_object_get_type(&db->cache[i]) &
         (BG_DB_FLAG_CONTAINER|BG_DB_FLAG_VCONTAINER)))
      {
      flush_object(db, &db->cache[i]);
      if(do_free)
        {
        if(db->cache[i].refcount)
          dead_refs++;
        bg_db_obj_cache_clear(&db->cache[i]);
        }
      }
    else
      db->cache[i].sync = 0;
    }
  
  while(!done)
    {
    done = 1;
    for(i = 0; i < db->cache_size; i++)
      {
      if(db->cache[i].sync)
        continue;
      
      done = 0;
      
      if(!children_in_cache(db, i))
        {
        flush_object(db, &db->cache[i]);
        if(do_free)
          {
          if(db->cache[i].refcount)
            {
            fprintf(stderr, "Dead reference (%d)\n", db->cache[i].refcount);
            bg_db_object_dump(&db->cache[i].obj);
            dead_refs++;
            }
          bg_db_obj_cache_clear(&db->cache[i]);
          }
        }
      }
    }

  if(do_free)
    {
    gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Cleaned up cache, %d dead references",
           dead_refs);

    dead_refs = 0;
    for(i = 0; i < db->cache_size; i++)
      {
      if(bg_db_object_is_valid(&db->cache[i].obj))
        {
        //   fprintf(stderr, "Object not freed:\n");
        //   bg_db_object_dump(&db->cache[i].obj);
        dead_refs++;
        }
      }
    gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "%d objects leaked",
           dead_refs);
    
    }

  
  }

void bg_db_flush(bg_db_t * db)
  {
  db_flush(db, 0);
  }

void bg_db_destroy(bg_db_t * db)
  {
  int num_added, num_deleted;
  db_flush(db, 0);
  /* Check if we need to clean things up */

  num_added = db->num_added;
  num_deleted = db->num_deleted;
  
  if(num_added || num_deleted)
    bg_db_cleanup_vfolders(db);

  if(num_deleted)
    {
    bg_db_cleanup_audio(db);
    bg_db_cleanup_video(db);
    }

  if(num_added || num_deleted)
    {
    bg_db_cleanup_tvseries(db);
    }
  
  /* Flush cache */
  db_flush(db, 1);
  free(db->cache);

  sqlite3_finalize(db->q_objects);
  sqlite3_finalize(db->q_files);
  sqlite3_finalize(db->q_audio_files);
  sqlite3_finalize(db->q_video_files);
  sqlite3_finalize(db->q_audio_albums);
  sqlite3_finalize(db->q_directories);
  sqlite3_finalize(db->q_video_infos);
  sqlite3_finalize(db->q_urls);
  sqlite3_finalize(db->q_rootfolders);
  
  sqlite3_close(db->db);
  free(db->base_dir);
  free(db->db_path);

  bg_db_string_cache_destroy(db->audio_artists);
  bg_db_string_cache_destroy(db->audio_genres);
  bg_db_string_cache_destroy(db->video_genres);  
  bg_db_string_cache_destroy(db->video_persons);
  bg_db_string_cache_destroy(db->video_countries);
  bg_db_string_cache_destroy(db->video_languages);

  bg_db_string_cache_destroy(db->audio_albums);
  bg_db_string_cache_destroy(db->mimetypes);

  bg_db_thumbnail_cache_free(&db->th_cache);
  
  free(db);
  }

/* Edit functions */
void bg_db_add_directory(bg_db_t * db, const char * d, int scan_flags)
  {
  bg_db_scan_item_t * files;
  int num_files = 0;
  char * path;
  int64_t id;
  
  path = bg_canonical_filename(d);
  
  if(strncmp(path, db->base_dir, db->base_len))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot add directory %s: No child of %s",
           path, db->base_dir);
    return;
    }

  bg_db_start_transaction(db);
  
  files = bg_db_scan_directory(path, &num_files);

  if((id = bg_dir_by_path(db, path)) > 0)
    {
    bg_db_dir_t * dir;
    dir = bg_db_object_query(db, id);
    
    gavl_log(GAVL_LOG_INFO, LOG_DOMAIN,
           "Directory %s already in database, updating", path);
    bg_db_update_files(db, files, num_files, dir->scan_flags, id, path);
    scan_flags = dir->scan_flags; // Ignore old scan flags
    id = bg_db_object_get_id(dir);
    bg_db_object_unref(dir);
    }
  else
    {
    /* Create root container */
    bg_db_scan_item_t item;
    id = -1;
    memset(&item, 0, sizeof(item));
    item.path = path;
    bg_db_dir_create(db, scan_flags,
                     &item, NULL, &id);
    bg_db_add_files(db, files, num_files, scan_flags, id);
    }
  bg_db_scan_items_free(files, num_files);

  bg_db_flush(db);
  
  /* Identify images */
  bg_db_identify_images(db, id, scan_flags);

  /* Identify .nfo */
  if(scan_flags & BG_DB_SCAN_VIDEO)
    bg_db_identify_nfo(db, id);

  bg_db_end_transaction(db);

  }

void bg_db_del_directory(bg_db_t * db, const char * d)
  {
  bg_db_dir_t * dir;
  
  int64_t id;
  char * path = bg_canonical_filename(d);
  
  if((id = bg_dir_by_path(db, path)) <= 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,
           "Directory %s not in database", path);
    free(path);
    return;
    }

  dir = bg_db_object_query(db, id);
  
  if(bg_db_object_get_parent(dir) > 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,
           "Directory %s not a root scan directory", path);
    bg_db_object_unref(dir);
    }
  else
    bg_db_object_delete(db, dir);
  free(path);
  }

int bg_db_date_equal(const bg_db_date_t * d1,
                     const bg_db_date_t * d2)
  {
  return
    (d1->year == d2->year) &&
    (d1->month == d2->month) &&
    (d1->day == d2->day);
  }

void bg_db_date_copy(bg_db_date_t * dst,
                     const bg_db_date_t * src)
  {
  memcpy(dst, src, sizeof(*dst));
  }

char * bg_db_date_to_string(const bg_db_date_t * d, char * str)
  {
  gavl_metadata_date_to_string(d->year, d->month, d->day, str);
  return str;
  }

void bg_db_string_to_date(const char * str, bg_db_date_t * d)
  {
  sscanf(str, "%d-%d-%d", &d->year, &d->month, &d->day);
  }

void bg_db_date_set_invalid(bg_db_date_t * d)
  {
  d->year  = 9999;
  d->day   = 01;
  d->month = 01;
  }

int bg_db_date_is_invalid(bg_db_date_t * d)
  {
  return (d->year == 9999) || (d->month == 0) || (d->day == 0);
  }

int64_t bg_db_cache_search(bg_db_t * db,
                           int (*compare)(const bg_db_object_t*,const void*),
                           const void * data)
  {
  int i;
  for(i = 0; i < db->cache_size; i++)
    {
    if(compare(&db->cache[i].obj.obj, data))
      return db->cache[i].obj.obj.id;
    }
  return -1;
  }

static const char * search_string_skip[] =
  {
    "a ",
    "the ",
    "der ",
    "die ",
    "das ",
    "ein ",
    "eine ",
    "le ",
    "les ",
    "la ",
    "l'",
    "os ",
    "'", // 'Round Midnight
    NULL
  };

const char * bg_db_get_search_string(const char * str)
  {
  int i, len;
  
  i = 0;
  while(search_string_skip[i])
    {
    len = strlen(search_string_skip[i]);
    if(!strncasecmp(str, search_string_skip[i], len))
      return str + len;
    i++;
    }
  return str;
  }

/* Get children */

void
bg_db_get_children(bg_db_t * db, int64_t id, bg_sqlite_id_tab_t * tab)
  {
  bg_db_object_type_t type;
  bg_db_object_t * parent = NULL;
  char * sql;

  bg_db_flush(db);
  parent = bg_db_object_query(db, id);
  if(!parent)
    return;
  type = bg_db_object_get_type(parent);

  if(!(type & (BG_DB_FLAG_CONTAINER|
               BG_DB_FLAG_VCONTAINER)))
    {
    goto fail;
    }
  else if(parent->klass && parent->klass->get_children)  // Custom function (e.g. virtual containers)
    parent->klass->get_children(db, parent, tab);
  else if(type & BG_DB_FLAG_CONTAINER)
    {
    sql = sqlite3_mprintf("select ID from OBJECTS where PARENT_ID = %"PRId64" ORDER BY LABEL COLLATE NOCASE;",
                          bg_db_object_get_id(parent));
    bg_sqlite_exec(db->db, sql, bg_sqlite_append_id_callback, tab);
    sqlite3_free(sql);
    }
  fail:
  if(parent)
    bg_db_object_unref(parent);
  
  }

/* Query children */

int
bg_db_query_children(bg_db_t * db, void * parent,
                     bg_db_query_callback cb, void * priv,
                     int start, int num, int * total_matches)
  {
  int i, ret = 0;
  int end;
  bg_db_obj_cache_t * ci = parent;
  void * obj;

  if(!ci->have_children)
    {
    bg_sqlite_id_tab_init(&ci->children);
    bg_db_get_children(db, bg_db_object_get_id(parent), &ci->children);
    }
  
  if(!ci->children.num_val)
    {
    if(total_matches)
      *total_matches = 0;
    return 0;
    }
  
  if(!num)
    end = ci->children.num_val;
  else
    {
    end = start + num;
    if(end > ci->children.num_val)
      end = ci->children.num_val;
    }

  if(total_matches)
    *total_matches = ci->children.num_val;
  
  for(i = start; i < end; i++)
    {
    obj = bg_db_object_query(db, ci->children.val[i]);
    if(!obj)
      {
      // Error
      goto fail;
      }
    cb(priv, obj);
    bg_db_object_unref(obj);
    ret++;
    }
  
  fail:
  return ret;
  }

int
bg_db_query_sql(bg_db_t * db, const char * sql, bg_db_query_callback cb, void * priv)
  {
  bg_sqlite_id_tab_t tab;
  int result;
  int i, ret = 0;
  void * obj;
  
  bg_sqlite_id_tab_init(&tab);
  bg_db_flush(db);

  result = bg_sqlite_exec(db->db, sql, bg_sqlite_append_id_callback, &tab);

  if(!result)
    goto fail;
  
  for(i = 0; i < tab.num_val; i++)
    {
    obj = bg_db_object_query(db, tab.val[i]);
    if(!obj)
      {
      // Error
      goto fail;
      }
    cb(priv, obj);
    bg_db_object_unref(obj);
    ret++;
    }
  
  fail:
  bg_sqlite_id_tab_free(&tab);

  return ret;
  
  }


/* Group management */

/* Changing these changes the database file */

static const struct
  {
  int id;
  char * label;
  }
groups[] =
  {
    { 1,  "0-9"    },
    { 2,  "A"      },
    { 3,  "B"      },
    { 4,  "C"      },
    { 5,  "D"      },
    { 6,  "E"      },
    { 7,  "F"      },
    { 8,  "G"      },
    { 9,  "H"      },
    { 10, "I"      },
    { 11, "J"      },
    { 12, "K"      },
    { 13, "L"      },
    { 14, "M"      },
    { 15, "N"      },
    { 16, "O"      },
    { 17, "P"      },
    { 18, "Q"      },
    { 19, "R"      },
    { 20, "S"      },
    { 21, "T"      },
    { 22, "U"      },
    { 23, "V"      },
    { 24, "W"      },
    { 25, "X"      },
    { 26, "Y"      },
    { 27, "Z"      },
    { 28, "~Others" },
  };

const char * bg_db_get_group(const char * str, int64_t * id)
  {
  char upper = toupper(*str);
  
  if((upper >= '0') && (upper <= '9'))
    *id = 1;
  else if((upper >= 'A') && (upper <= 'Z'))
    *id = upper - 'A' + 2;
  else
    *id = 28;
  return groups[*id - 1].label;
  }

char * bg_db_get_group_condition(const char * row, int group_id)
  {
  if(group_id == 1)
    return bg_sprintf("(UPPER(%s) GLOB '[0-9]*')", row);
  else if(group_id == 28)
    return bg_sprintf("(UPPER(%s) NOT GLOB '[A-Z0-9]*')", row);
  else
    return bg_sprintf("(UPPER(%s) GLOB '%c*')", row, 'A' + group_id-2);
  }

int64_t bg_db_object_by_location(bg_db_t * db, const char * location)

  {
  if(!strncasecmp(location, "http://", 7) ||
     !strncasecmp(location, "https://", 8) ||
     !strncasecmp(location, "mms://", 6))
    {
    return bg_db_url_by_location(db, location);
    }
  else if(!strncasecmp(location, "id://", 5))
    {
    if(isdigit(location[5]))
      return strtoll(location + 5, NULL, 10);
    else
      return -1;
    }
  return
    bg_db_file_by_path(db, location);
  
  }
