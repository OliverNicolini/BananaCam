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

int		auto_focus(t_cam *c, char **param)
{
  CameraWidget	*widget = NULL;
  CameraWidget	*child = NULL;
  int		ret;
  int		val;
  CameraWidgetType	type;

  param = param;

  ret = gp_camera_get_config(c->camera, &widget, c->context);

  if (ret < GP_OK)
    {
      creat_and_send_message(KO, NULL, NULL, "camera_get_config failed", c);
      gp_widget_free (widget); return (GP_ERROR);
    }

  ret = lookup_widget (widget, "manualfocusdrive", &child);
  if (ret < GP_OK)
    {
      creat_and_send_message(KO, NULL, NULL, "lookup 'autofocus driver' failed", c);
      gp_widget_free (widget); return (GP_ERROR);
    }

  ret = gp_widget_get_type (child, &type);
  if (ret < GP_OK)
    {
      creat_and_send_message(KO, NULL, NULL, "widget get type failed", c);
      gp_widget_free (widget); return (GP_ERROR);
    }

  if (type != GP_WIDGET_TOGGLE)
    {
      creat_and_send_message(BAD_PARAMETERS, NULL, NULL, "widget has bad type", c);
      gp_widget_free (widget); return (GP_ERROR);
    }

  ret = gp_widget_get_value (child, &val);
  if (ret < GP_OK)
    {
      creat_and_send_message(KO, NULL, NULL, "could not get widget value", c);
      gp_widget_free (widget); return (GP_ERROR);
    }
  val = -1;
  ret = gp_widget_set_value (child, &val);
  if (ret < GP_OK)
    {
      creat_and_send_message(KO, NULL, NULL, "could not set widget value to 1", c);
      gp_widget_free (widget); return (GP_ERROR);
    }

  ret = gp_camera_set_config (c->camera, widget, c->context);
  if (ret < GP_OK)
    {
      creat_and_send_message(KO, NULL, NULL, "could not set config tree to autofocus", c);
      gp_widget_free (widget); return (GP_ERROR);
    }

  gp_widget_free (widget);
  creat_and_send_message(COMPLETE, NULL, NULL, "autofocus done", c);
  return (GP_OK);
}

/*
int		manual_focus(t_cam *c, char **param)
{
  CameraWidget		*widget = NULL, *child = NULL;
  CameraWidgetType	type;
  int			ret;
  float			rval;
  char			*mval;
  int			xx = 0;
  int			choices = 0;

  if (param)
    {
      if (param[0])
	xx = atoi(param[0]);
    }
  ret = gp_camera_get_config (c->camera, &widget, c->context);
  if (ret < GP_OK) {
    fprintf (stderr, "camera_get_config failed: %d\n", ret);
    return ret;
  }
  ret = lookup_widget (widget, "manualfocusdrive", &child);
  if (ret < GP_OK) {
    fprintf (stderr, "lookup 'manualfocusdrive' failed: %d\n", ret);
    goto out;
  }


  ret = gp_widget_get_type (child, &type);
  if (ret < GP_OK) {
    fprintf (stderr, "widget get type failed: %d\n", ret);
    goto out;
  }
  switch (type) {
  case GP_WIDGET_RADIO: {
    choices = gp_widget_count_choices (child);

    ret = gp_widget_get_value (child, &mval);
    if (ret < GP_OK) {
      fprintf (stderr, "could not get widget value: %d\n", ret);
      goto out;
    }
    if (choices == 7) {
      ret = gp_widget_get_choice (child, xx+4, (const char**)&mval);
      if (ret < GP_OK) {
	fprintf (stderr, "could not get widget choice %d: %d\n", xx+2, ret);
	goto out;
      }
      fprintf(stderr,"manual focus %d -> %s\n", xx, mval);
    }
    ret = gp_widget_set_value (child, mval);
    if (ret < GP_OK) {
      fprintf (stderr, "could not set widget value to 1: %d\n", ret);
      goto out;
    }
    break;
  }
  case GP_WIDGET_RANGE:
    ret = gp_widget_get_value (child, &rval);
    if (ret < GP_OK) {
      fprintf (stderr, "could not get widget value: %d\n", ret);
      goto out;
    }

    switch (xx) {
    case -3:	rval = -1024;break;
    case -2:	rval =  -512;break;
    case -1:	rval =  -128;break;
    case  0:	rval =     0;break;
    case  1:	rval =   128;break;
    case  2:	rval =   512;break;
    case  3:	rval =  1024;break;

    default:	rval = xx;	break;
    }

    fprintf(stderr,"manual focus %d -> %f\n", xx, rval);

    ret = gp_widget_set_value (child, &rval);
    if (ret < GP_OK) {
      fprintf (stderr, "could not set widget value to 1: %d\n", ret);
      goto out;
    }
    break;
  default:
    fprintf (stderr, "widget has bad type %d\n", type);
    ret = GP_ERROR_BAD_PARAMETERS;
    goto out;
  }


  ret = gp_camera_set_config (c->camera, widget, c->context);
  if (ret < GP_OK) {
    fprintf (stderr, "could not set config tree to autofocus: %d\n", ret);
    goto out;
  }
 out:
  gp_widget_free (widget);
  return ret;
}
*/


