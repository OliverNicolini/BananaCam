/*
**
** This file is part of BananaCam.
**
** BananaCam is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** BananaCam is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with BananaCam.  If not, see <http://www.gnu.org/licenses/>.
**
*/

#include "camera_control.h"

int		set_get_config_init(char *key, CameraWidget **widget,
				    CameraWidget **child, t_cam *c)
{
  CameraWidget		*_widget = NULL, *_child = NULL;
  CameraWidgetType	type;
  int			ret;

  ret = gp_camera_get_config(c->camera, &_widget, c->context);
  if (ret < GP_OK)
    {
      creat_and_send_message(KO, NULL, NULL, "camera_get_config failed", c);
      gp_widget_free(_widget); return (GP_ERROR);
    }

  ret = lookup_widget(_widget, key, &_child);
  if (ret < GP_OK)
    {
      creat_and_send_message(KO, NULL, NULL, "lookup 'driver' failed", c);
      gp_widget_free(_widget); return (GP_ERROR);
    }

  ret = gp_widget_get_type(_child, &type);
  if (ret < GP_OK)
    {
      creat_and_send_message(KO, NULL, NULL, "widget get type failed", c);
      gp_widget_free(_widget); return (GP_ERROR);
    }

  if (type != GP_WIDGET_MENU && type != GP_WIDGET_RADIO && type != GP_WIDGET_TEXT)
    {
      creat_and_send_message(KO, NULL, NULL, "widget has bad type", c);
      gp_widget_free(_widget); return (GP_ERROR);
    }

  *widget = _widget;
  *child = _child;

  return (GP_OK);
}

int		_set_get_config_init(char *key, CameraWidget **widget,
				    CameraWidget **child, t_cam *c)
{
  CameraWidget		*_widget = NULL, *_child = NULL;
  CameraWidgetType	type;
  int			ret;

  ret = gp_camera_get_config(c->camera, &_widget, c->context);
  if (ret < GP_OK)
    gp_widget_free(_widget); return (GP_ERROR);

  ret = lookup_widget(_widget, key, &_child);
  if (ret < GP_OK)
    gp_widget_free(_widget); return (GP_ERROR);

  ret = gp_widget_get_type(_child, &type);
  if (ret < GP_OK)
    gp_widget_free(_widget); return (GP_ERROR);

  if (type != GP_WIDGET_MENU && type != GP_WIDGET_RADIO && type != GP_WIDGET_TEXT)
    gp_widget_free(_widget); return (GP_ERROR);

  *widget = _widget;
  *child = _child;

  return (GP_OK);
}

char		*get_widget_children(t_cam *c, CameraWidget *widget, char *info)
{
  int		i = 0;
  int		widget_nb = 0;
  int		child_nb = 0;
  CameraWidget	*child = NULL;
  char		*choices;
  const char	*name;
  char		*full_info = NULL;
  char		*full_info_tmp = NULL;

  widget_nb = gp_widget_count_children(widget);
  gp_widget_get_name(widget, &name);
  while (i < widget_nb)
    {
      gp_widget_get_child(widget, i, &child);
      gp_widget_get_name(child, &name);
      child_nb = gp_widget_count_children(child);
      if (child_nb > 0)
	{
	  full_info_tmp = get_widget_children(c, child, full_info);
	  asprintf(&full_info, "%s%s", info, full_info_tmp);
	}
      else
	{
	  if (full_info != NULL)
	    {
	      full_info_tmp = strdup(full_info);
	      free(full_info);
	      asprintf(&full_info, "%s%s=>", full_info_tmp, name);
	      free(full_info_tmp);
	    }
	  else
	    asprintf(&full_info, "%s=>", name);
	  choices = get_widget_choices(c, child);
	  asprintf(&full_info_tmp, "%s%s\n", full_info, choices);
	  free(full_info);
	  full_info = strdup(full_info_tmp);
	  free(full_info_tmp);
	  }
      i++;
    }
  asprintf(&full_info_tmp, "%s%s", info, full_info);
  return (full_info_tmp);
}


char		*get_widget_choices(t_cam *c, CameraWidget *widget)
{
  int		j = 0;
  const char	*choice;
  int		choices_nb = 0;
  char		*full_choices = NULL;
  char		*full_choices_tmp = NULL;
  CameraWidgetType	type;
  float		min, max, increment;

  c = c;
  gp_widget_get_type(widget, &type);
  if (type == GP_WIDGET_RANGE)
    {
      full_choices_tmp = strdup("range#");
      gp_widget_get_range(widget, &min, &max, &increment);
      asprintf(&full_choices, "%s%f|%f|%f", full_choices_tmp, min, max, increment);
      free(full_choices_tmp);
      return (full_choices);
    }
  else if (type == GP_WIDGET_TEXT)
    return (strdup("text#"));
  else if (type == GP_WIDGET_DATE)
    return (strdup("date#"));
  else if (type == GP_WIDGET_TOGGLE)
    return (strdup("toggle#"));
  else if (type == GP_WIDGET_BUTTON)
    return (strdup("button#"));
  else if (type == GP_WIDGET_RADIO)
    full_choices = strdup("radio#");
  else if (type == GP_WIDGET_MENU)
    full_choices = strdup("menu#");

  choices_nb = gp_widget_count_choices(widget);
  while (j < choices_nb)
    {
      gp_widget_get_choice(widget, j, &choice);
      if (strncmp(choice, "Unknown", 7) != 0)
	{
	  if (full_choices != NULL)
	    {
	      full_choices_tmp = strdup(full_choices);
	      free(full_choices);
	      asprintf(&full_choices, "%s%s|", full_choices_tmp, choice);
	      free(full_choices_tmp);
	    }
	  else
	    asprintf(&full_choices, "%s|", choice);
	}
      j++;
    }
  return (full_choices);
}

int		get_all_widget_and_choices(t_cam *c)
{
  CameraWidget	*widget = NULL;
  char		*full_info;

  if (gp_camera_get_config(c->camera, &widget, c->context) < GP_OK)
    {
      creat_and_send_message(KO, NULL, NULL, "camera_get_config failed", c);
      gp_widget_free(widget); return (GP_ERROR);
    }

  full_info = get_widget_children(c, widget, "");
  c->camera_value_list = full_info;

  return (GP_OK);
}

int		_get_config(char *key, char **val, t_cam *c)
{
  CameraWidget		*widget = NULL;
  CameraWidget		*child = NULL;
  int			ret;

  if (_set_get_config_init(key, &widget, &child, c) == GP_ERROR)
    return (GP_ERROR);

  ret = gp_widget_get_value (child, val);
  if (ret < GP_OK) {
    gp_widget_free(widget); return (GP_ERROR);
  }

  gp_widget_free (widget);
  return (GP_OK);
}

int		get_config(char *key, t_cam *c)
{
  CameraWidget		*widget = NULL;
  CameraWidget		*child = NULL;
  int			ret;
  char			*val;
  char			*msg;
  char			**param;

  if (set_get_config_init(key, &widget, &child, c) == GP_ERROR)
    return (GP_ERROR);

  ret = gp_widget_get_value (child, &val);
  if (ret < GP_OK) {
    asprintf(&msg, "could not query %s value", key);
    creat_and_send_message(KO, NULL, NULL, msg, c);
    free(msg);
    gp_widget_free(widget); return (GP_ERROR);
  }

  param = malloc(sizeof(*param));
  param[0] = strdup(val);
  param[1] = NULL;
  asprintf(&msg, "%s=%s", key, val);
  creat_and_send_message(VALUE, key, param, msg, c);
  free(msg);
  asprintf(&msg, "getting of %s", key);
  creat_and_send_message(COMPLETE, NULL, NULL, msg, c);
  free(msg);
  if (param && param[0])
    {
      free(param[0]);
      free(param);
    }

  gp_widget_free (widget);
  return (GP_OK);
}

int		set_config(char *key, char *val, t_cam *c)
{
  CameraWidget		*widget = NULL, *child = NULL;
  int			ret;
  char			*msg;

  if (set_get_config_init(key, &widget, &child, c) == GP_ERROR)
    return (GP_ERROR);

  ret = gp_widget_set_value(child, val);
  if (ret < GP_OK)
    {
      creat_and_send_message(KO, NULL, NULL, "could not set widget value", c);
      gp_widget_free(widget); return (GP_ERROR);
    }

  ret = gp_camera_set_config(c->camera, widget, c->context);
  if (ret < GP_OK)
    {
      creat_and_send_message(KO, NULL, NULL, "could not set config tree to autofocus", c);
      gp_widget_free(widget); return (GP_ERROR);
    }

  gp_widget_free(widget);

  asprintf(&msg, "setting of %s to %s done", key, val);
  creat_and_send_message(COMPLETE, NULL, NULL, msg, c);
  free(msg);
  return (GP_OK);
}

int		lookup_widget(CameraWidget*widget, const char *key, CameraWidget **child)
{
  int ret;

  ret = gp_widget_get_child_by_name (widget, key, child);
  if (ret < GP_OK)
    ret = gp_widget_get_child_by_label (widget, key, child);
  return ret;
}
