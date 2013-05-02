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

char		*creat_message(int code, char *command, char **param, char *message)
{
  char		*final_message = NULL;
  char		*tmp;
  int		i = 0;

  asprintf(&tmp, "%i|%s|%s", code, (message == NULL ? "NULL" : message),
		 (command == NULL ? "NULL" : command));
  final_message = tmp;
  if (param != NULL)
    {
      while (param[i] != NULL)
	{
	  asprintf(&final_message, "%s|%s|", tmp, param[i]);
	  tmp = final_message;
	  i++;
	}
    }
  return (final_message);
}

void		creat_and_send_message(int code, char *command,
				       char **param, char *message, t_cam *c)
{
  char		*msg = NULL;

  msg = creat_message(code, command, param, message);
  write(c->active_sock, msg, strlen(msg));
}

char		**parse_message(char *comm_protocol, int *code, char **command, char **message)
{
  char		**tmp;
  char		*save;

  tmp = str_to_wordtab(comm_protocol, '|');

  if (tmp == NULL)
    return (NULL);

  if (tmp[0])
    {
      *code = atoi(tmp[0]);
      save = tmp[0];
      tmp = &tmp[1];
      free(save);
    }
  if (tmp[0])
    {
      *command = strdup(tmp[0]);
      save = tmp[0];
      tmp = &tmp[1];
      free(save);
    }
  if (tmp[0])
    {
      *message = strdup(tmp[0]);
      save = tmp[0];
      tmp = &tmp[1];
      free(save);
    }
  return (tmp);
}

void		add_client_to_list(t_serv_comm *s, int socket,
				   struct sockaddr_un client_addr)
{
  t_serv_clients *c;

  if ((c = s->first_client) == NULL)
    {
      s->first_client = malloc(sizeof(*s->first_client));
      c = s->first_client;
    }
  else
    {
      while (c->next != NULL)
	c = c->next;
      c->next = malloc(sizeof(*c->next));
      c = c->next;
    }
  c->next = NULL;
  c->sock = socket;
  c->client_addr = client_addr;
}

void		add_client_to_sock_monitor(t_serv_comm *s, int socket)
{
  if (s->bigger_fd < socket)
    s->bigger_fd = socket;
  FD_SET(socket, &(s->rd_fds));
}

void		serv_accept_new_connections(t_serv_comm *s)
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
  if (s->c->camera_value_list)
    write(socket, s->c->camera_value_list, strlen(s->c->camera_value_list));
}

void		client_diconnected(t_serv_comm *s, int sock)
{
  t_serv_clients *c;
  t_serv_clients *tmp;

  s->first_client = 0;
  s->bigger_fd = s->sock_serv;
  pthread_cond_signal(&s->c->liveview_condvar);

  /*  c = s->first_client;
  if (c)
    {
      if (c->sock == sock)
	{
	  free(s->first_client);
	  s->first_client = NULL;
	  c = NULL;
	}
    }
  c = s->first_client;
  while (c != NULL)
    {
      if (c->sock == sock)
	{
	  tmp->next = c->next;
	  free(c);
	  break;
	}
      tmp = c;
      c = c->next;
      }*/
}

void	       message_queue_push(char *command, char *msg, char **param, int code, t_cam *c)
{
  t_message_queue *tmp;

  if (c->first_msg == NULL)
    {
      c->first_msg = malloc(sizeof(*c->first_msg));
      c->first_msg->next = NULL;
      c->first_msg->command = command;
      c->first_msg->msg = msg;
      c->first_msg->param = param;
      c->first_msg->code = code;
    }
  else
    {
      tmp = c->first_msg;
      while (tmp->next != NULL)
	tmp = tmp->next;
      tmp->next = malloc(sizeof(*tmp->next));
      tmp = tmp->next;
      tmp->next = NULL;
      tmp->command = command;
      tmp->msg = msg;
      tmp->param = param;
      tmp->code = code;
    }
}

t_message_queue	*message_queue_pop(t_cam *c)
{
  t_message_queue *tmp;
  if (c->first_msg != NULL)
    {
      tmp = c->first_msg;
      c->first_msg = c->first_msg->next;
      return (tmp);
    }
  else
    return (NULL);
}

void		parse_and_push_message(char *buff, t_cam *c)
{
  int		code = 0;
  char		*message = NULL;
  char		*command = NULL;
  char		**param = NULL;
  int		i = 0;
  char		**msg;

  msg = str_to_wordtab(buff, '\n');
  while (msg && msg[i] != NULL)
    {
      param = parse_message(msg[i], &code, &command, &message);
      message_queue_push(command, message, param, code, c);
      i++;
    }
}

void		interpret_and_exec(char *buff, t_cam *c)
{
  int		code = 0;
  char		*message = NULL;
  char		*command = NULL;
  char		**param = NULL;
  int		i = 0;
  t_message_queue *tmp;

  while (c->first_msg != NULL)
    {
      tmp = message_queue_pop(c);

      if (tmp->code == OK)
	printf("OK:\t\t%s\n", (tmp->msg == NULL ? " NULL" : tmp->msg));
      else if (tmp->code == KO)
	printf("KO:\t\t%s\n", (tmp->msg == NULL ? " NULL" : tmp->msg));
      else if (tmp->code == WRONG_COMMAND)
	printf("WRONG_COMMAND:\t\t%s", (tmp->msg == NULL ? " NULL" : tmp->msg));
      else if (tmp->code == BAD_PARAMETERS)
	{
	  printf("OK:\t\t%s | bad param: ", (tmp->msg == NULL ? " NULL" : tmp->msg));
	  while (tmp->param[i] != NULL)
	    {
	      printf("%s | ", tmp->param[i]);
	      i++;
	    }
	  printf("\n");
	}
      else if (tmp->code == WAIT_RESPONSE)
	printf("WAIT_RESPONSE:\t\t%s", (tmp->msg == NULL ? " NULL" : tmp->msg));
      else if (tmp->code == COMPLETE)
	printf("COMPLETE:\t\t%s", (tmp->msg == NULL ? " NULL" : tmp->msg));
      else if (tmp->code == EXEC)
	{
	  i = 0;
	  printf("EXEC:\t\t%s ==> ", (tmp->command == NULL ? "NULL" : tmp->command));
	  while (tmp->param[i] != NULL)
	    {
	      printf("%s | ", tmp->param[i]);
	      i++;
	    }
	  printf("## %s\n", (tmp->msg == NULL ? " NULL" : tmp->msg));

	  if (tmp->command)
	    exec_command(c, tmp->command, tmp->param);

	}
      if (tmp->code == 0)
	printf("//// UNKNOWN COMMAND \\\\\n");
    }
}

void		check_modified_fd(t_serv_comm *s)
{
  t_serv_clients *c;
  char		buff[4096];
  int		len;

  if (FD_ISSET(s->sock_serv, &(s->rd_fds)))
      serv_accept_new_connections(s);
  else
    {
      c = s->first_client;
      while (c != NULL)
	{
	  if (FD_ISSET(c->sock, &(s->rd_fds)))
	    {
	      s->c->active_sock = c->sock;
	      len = read(c->sock, buff, 4096);
	      if (len == 0)
		{
		  client_diconnected(s, c->sock);
		  break;
		}
	      if (buff[len - 1] == '\n')
		buff[len - 1] = '\0';
	      else
		buff[len] = '\0';
	      parse_and_push_message(buff, s->c);
	      interpret_and_exec(buff, s->c);
	      s->c->active_sock = 0;
	      break;
	    }
	  c = c->next;
	}
    }
}

void		reset_set_fd_to_monitor(t_serv_comm *s)
{
  t_serv_clients *c = NULL;

  if (s == NULL) return;
  s->bigger_fd = 0;
  FD_ZERO(&(s->rd_fds));
  FD_SET(s->sock_serv, &(s->rd_fds));
  if (s->sock_serv > s->bigger_fd)
    s->bigger_fd = s->sock_serv;
  c = s->first_client;
  while (c != NULL)
    {
      FD_SET(c->sock, &(s->rd_fds));
      if (c->sock > s->bigger_fd)
	s->bigger_fd = c->sock;
      c = c->next;
    }
}

void		serv_working_loop(t_serv_comm *s)
{
  s->state = ON;
  add_client_to_sock_monitor(s, s->sock_serv);
  while (s->state != OFF && s->state != FAIL)
    {
      reset_set_fd_to_monitor(s);
      if (select((s->bigger_fd + 1), &(s->rd_fds),
		  NULL, NULL, NULL) == -1)
	perror("select comm");
      check_modified_fd(s);
    }
}

int		serv_init_connect(t_serv_comm *s, char *path)
{
  if ((s->sock_serv = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
      fprintf(stderr, "Socket creation Error");
      return (-1);
    }
  s->queue_size = 50;
  unlink(path);
  memset(&(s->serv_addr), 0, sizeof(s->serv_addr));
  s->serv_addr.sun_family = AF_UNIX;
  snprintf(s->serv_addr.sun_path, 108, "%s", path);
  if (bind(s->sock_serv, (struct sockaddr*)&(s->serv_addr),
	   sizeof(s->serv_addr)) < 0)
    {
      fprintf(stderr, "Bind Error");
      return (-1);
    }
  chmod(UNIX_SOCKET_PATH, 0666);
  listen(s->sock_serv, s->queue_size);
  return (0);
}

void		*init_comm(t_cam *c, char *path)
{
  t_serv_comm	*s;

  s = malloc(sizeof(*s));
  c->sock_struct = s;
  s->c = c;
  s->bigger_fd = 0;
  s->first_client = NULL;
  serv_init_connect(s, path);
  serv_working_loop(s);
  return (NULL);
}

