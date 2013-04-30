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

int		set_default_folder_path(t_cam *c, char **param)
{
  if (param)
    if (param[0])
      c->folder_path = strdup(param[0]);
  creat_and_send_message(COMPLETE, NULL, NULL, "setting of default path", c);
}

int		get_default_folder_path(t_cam *c, char **p)
{
  char		*msg;
  char		**param;

  p = p;
  param = malloc(sizeof(*param));
  param[0] = strdup(c->folder_path);
  param[1] = NULL;
  asprintf(&msg, "defaultpath=%s", c->folder_path);
  creat_and_send_message(VALUE, "defaultpath", param, msg, c);
  free(msg);
  if (param && param[0])
    {
      free(param[0]);
      free(param);
    }
  creat_and_send_message(COMPLETE, NULL, NULL, "getting of default path", c);
  return (0);
}
