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

void		interpret_header_tcp(char *buff, int sock, t_serv_comm *s)
{
  int		i;
  int		j;
  char		**tab;
  char		**tab2;
  char		*final_msg;

  if (buff == NULL)
    {
      close(sock);
      return;
    }
  tab = str_to_wordtab(buff, '\n');
  if (tab != NULL && strncmp(tab[0], "GET /?BananaCam=", 16) != 0)
    {
      close(sock);
      return;
    }

  tab2 = str_to_wordtab(tab[0], '=');
  tab = str_to_wordtab(tab2[1], ' ');
  tab2 = str_to_wordtab(tab[0], '&');

  asprintf(&final_msg, "%s", tab2[0]);

  i = 1;
  while (tab2[i] != NULL)
    {
      asprintf(&final_msg, "%s|%s", final_msg, tab2[i]);
      i++;
    }

  asprintf(&final_msg,  "%s|\n", final_msg);

  parse_and_push_message(final_msg, s->c);
  interpret_and_exec(final_msg, s->c);

  close(sock);
}

void		serv_accept_new_connections_tcp(t_serv_comm *s)
{
  int		socket;
  struct sockaddr_in	client_addr;
  socklen_t	len;
  char		buff[4096];
  int		l;

  l = 0;
  len = sizeof(client_addr);
  if ((socket = accept(s->sock_serv, (struct sockaddr*)&client_addr,
		       &len)) == -1)
    {
      printf("Accept Error\n");
      s->state = FAIL;
      return;
    }
  l = read(socket, buff, 4096);
  if (l != 0)
    {
      buff[l] = '\0';
      interpret_header_tcp(buff, socket, s);
    }
}

void		serv_working_loop_tcp(t_serv_comm *s)
{
  s->state = ON;
  while (s->state != OFF && s->state != FAIL)
    {
      reset_set_fd_to_monitor(s);
      if (select((s->bigger_fd + 1), &(s->rd_fds),
		 NULL, NULL, NULL) == -1)
	{
	  printf("tcp select failed\n");
	  s->state = FAIL;
	}
      if (FD_ISSET(s->sock_serv, &(s->rd_fds)))
	serv_accept_new_connections_tcp(s);
    }
}

int		serv_init_connect_tcp(t_serv_comm *s, int port)
{
  int		yes;

  yes = 1;
  if ((s->sock_serv = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      printf("Socket creation Error\n");
      return (-1);
    }
  memset(&(s->serv_addr_tcp), 0, sizeof(s->serv_addr_tcp));
  s->serv_addr_tcp.sin_family = AF_INET;
  s->serv_addr_tcp.sin_addr.s_addr = INADDR_ANY;
  s->serv_addr_tcp.sin_port = htons(port);

  if (setsockopt(s->sock_serv, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    perror("setsockopt");
    exit(1);
  }

  if (bind(s->sock_serv, (struct sockaddr*)&(s->serv_addr_tcp),
	   sizeof(s->serv_addr_tcp)) < 0)
    {
      printf("Bind Error\n");
      return (-1);
    }
  listen(s->sock_serv, 75);
  return (0);
}

void		*init_comm_tcp(t_cam *c, int port)
{
  t_serv_comm	*s;

  s = malloc(sizeof(*s));
  c->sock_struct_tcp = s;
  s->c = c;
  s->bigger_fd = 0;
  s->first_client = NULL;
  serv_init_connect_tcp(s, port);
  serv_working_loop_tcp(s);
  return (NULL);
}
