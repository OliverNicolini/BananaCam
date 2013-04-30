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

int		liveviewfps(t_cam *c, char **param)
{
  if (param)
    if (param[0])
      {
	c->liveview_fps = atoi(param[0]);
	c->liveview_fps_time = 1000000 / c->liveview_fps;
      }
  creat_and_send_message(COMPLETE, NULL, NULL, "Live View FPS set", c);
  return (0);
}

int		get_liveviewfps(t_cam *c, char **p)
{
  char		*msg;
  char		**param;

  p = p;
  param = malloc(sizeof(*param));
  asprintf(&param[0], "%i", c->liveview_fps);
  param[1] = NULL;
  asprintf(&msg, "liveviewfps=%i", c->liveview_fps);
  creat_and_send_message(VALUE, "liveviewfps", param, msg, c);
  free(msg);
  if (param && param[0])
    {
      free(param[0]);
      free(param);
    }
  creat_and_send_message(COMPLETE, NULL, NULL, "getting of liveviewfps", c);
  return (0);
}

int		liveview(t_cam *c, char **param)
{
  param = param;
  c->liveview = (c->liveview == 0 ? 1 : 0);
  pthread_cond_signal(&c->liveview_condvar);
  return (0);
}

void		*liveview_launcher(void *a)
{
  t_cam		*c;

#ifndef __APPLE__

  prctl(PR_SET_NAME, "Liveview_data");

#endif

  c = (t_cam*)a;
  init_data_comm(c, UNIX_SOCKET_DATA_PATH);
  return (NULL);
}
