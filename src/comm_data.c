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

void		serv_accept_new_connections_data(t_serv_comm *s)
{
  int		socket;
  struct sockaddr_un	client_addr;
  socklen_t	len;

  len = sizeof(client_addr);
  if ((socket = accept(s->sock_serv, (struct sockaddr*)&client_addr,
		       &len)) == -1)
    {
      fprintf(stderr, "Accept Error");
      s->state = FAIL;
      return;
    }
  add_client_to_list(s, socket, client_addr);
  add_client_to_sock_monitor(s, socket);
}

void		check_time(t_cam *c, struct timeval *start_time, struct timeval *end_time)
{
  struct timeval difference;

  difference.tv_sec = end_time->tv_sec - start_time->tv_sec ;
  difference.tv_usec = end_time->tv_usec - start_time->tv_usec;

  /* Using while instead of if below makes the code slightly more robust. */

  while(difference.tv_usec < 0)
    {
      difference.tv_usec += 1000000;
      difference.tv_sec -= 1;
    }
  if (difference.tv_sec == 0)
    {
      if (difference.tv_usec < c->liveview_fps_time)
	usleep(c->liveview_fps_time - difference.tv_usec);
    }
}

void		data_serv_working_loop(t_serv_comm *s)
{
  CameraFile    *file;
  const char	*data;
  char		*header;
  unsigned long	size;
  int		len = 0;
  struct timeval time_beggining;
  struct timeval time_final;

  s->first_client = NULL;

  FD_ZERO(&(s->rd_fds));
  FD_SET(s->sock_serv, &(s->rd_fds));
  if (s->sock_serv > s->bigger_fd)
    s->bigger_fd = s->sock_serv;

  if (select((s->sock_serv + 1), &(s->rd_fds),
	     NULL, NULL, NULL) == -1)
    perror("select data");

  if (FD_ISSET(s->sock_serv, &(s->rd_fds)))
      serv_accept_new_connections_data(s);

  gp_file_new(&file);

  while (1)
    {
      gettimeofday(&time_beggining, NULL);
      if (!s->c->liveview)
	{
	  if (len != 0)
	    gp_camera_exit(s->c->camera, s->c->context);
	  pthread_mutex_lock(&s->c->liveview_mutex);
	  pthread_cond_wait(&s->c->liveview_condvar, &s->c->liveview_mutex);
	  pthread_mutex_unlock(&s->c->liveview_mutex);
	}
      gp_camera_capture_preview(s->c->camera, file, s->c->context);
      gp_file_get_data_and_size(file, &data, &size);
      len = asprintf(&header, "||>>%li>>||", size);
      gettimeofday(&time_final, NULL);
      check_time(s->c, &time_beggining, &time_final);

      if (write(s->first_client->sock, header, len) < 0)
	{
	  s->c->liveview = 0;
	  gp_camera_exit(s->c->camera, s->c->context);
	  client_diconnected(s, s->first_client->sock);
	  return;
	}
      if (write(s->first_client->sock, data, size) < 0)
	{
	  s->c->liveview = 0;
	  gp_camera_exit(s->c->camera, s->c->context);
	  client_diconnected(s, s->first_client->sock);
	  return;
	}
      free(header);
    }
}

void		*init_data_comm(t_cam *c, char *path)
{
  t_serv_comm	*s;

  s = malloc(sizeof(*s));
  c->data_sock_struct = s;
  s->c = c;
  s->bigger_fd = 0;
  s->first_client = NULL;
  serv_init_connect(s, path);
  while (1)
    {
      s->bigger_fd = 0;
      data_serv_working_loop(s);
    }
  return (NULL);
}

