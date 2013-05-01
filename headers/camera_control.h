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

#ifndef __CAMERA_CONTROL_H__
#define __CAMERA_CONTROL_H__

#include <pthread.h>

#ifndef __APPLE__

#include <sys/prctl.h>

#endif

#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <gphoto2/gphoto2.h>
#include <gphoto2/gphoto2-camera.h>

#include "typedef.h"

struct		s_func
{
  char		*name;
  int		(*func_ptr)(t_cam *c, char **param);
  t_func	*next;
};

struct		s_cam
{
  Camera	*camera;
  GPContext	*context;
  int		ret;
  char		*folder_path;
  int		active_sock;
  t_serv_comm	*sock_struct;
  t_serv_comm	*data_sock_struct;
  t_func	*first_func_ptr;
  char		*camera_value_list;

  pthread_mutex_t	liveview_mutex;
  pthread_cond_t	liveview_condvar;
  pthread_t		liveview_thread;
  int			liveview;
  int			liveview_fps;
  int			liveview_fps_time;
};

struct			s_serv_comm
{
  int			sock_serv;
  struct sockaddr_un	serv_addr;
  int			queue_size;
  int			state;
  fd_set		rd_fds;
  int			bigger_fd;
  t_serv_clients	*first_client;
  t_cam			*c;
};

struct			s_serv_clients
{
  int			sock;
  struct sockaddr_un	client_addr;
  t_serv_clients	*next;
};

#define	ON	1
#define	OFF	0
#define FAIL	-1

#define UNIX_SOCKET_PATH "/tmp/camera_control.sock"
#define UNIX_SOCKET_DATA_PATH "/tmp/camera_control_data.sock"


#define OK		1
#define KO		2
#define WRONG_COMMAND	4
#define BAD_PARAMETERS	8
#define WAIT_RESPONSE	16
#define COMPLETE	32
#define EXEC		64
#define INFO		128
#define VALUE		256

/*		main			*/

void		*init_comm(t_cam *c, char *path);
int		exec_command(t_cam *c, char *command, char **param);

/*		comm			*/

char		*creat_message(int code, char *command, char **param, char *message);
char		**parse_message(char *comm_protocol, int *code, char **command, char **message);
void		creat_and_send_message(int code, char *command,
				       char **param, char *message, t_cam *c);
int		serv_init_connect(t_serv_comm *s, char *path);
void		serv_working_loop(t_serv_comm *s);
void		reset_set_fd_to_monitor(t_serv_comm *s);
void		check_modified_fd(t_serv_comm *s);
void		client_diconnected(t_serv_comm *s, int sock);
void		serv_accept_new_connections(t_serv_comm *s);
void		add_client_to_sock_monitor(t_serv_comm *s, int socket);
void		add_client_to_list(t_serv_comm *s, int socket,
				   struct sockaddr_un client_addr);

/*		data_comm		*/

void		serv_accept_new_connections_data(t_serv_comm *s);
void		check_time(t_cam *c, struct timeval *start_time, struct timeval *end_time);
void		data_serv_working_loop(t_serv_comm *s);
void		*init_data_comm(t_cam *c, char *path);

/*		autofocus		*/

int		auto_focus(t_cam *c, char **param);
int		manual_focus(t_cam *c, char **param);

/*		utils			*/

char		**str_to_wordtab(char *buff, char c);
int		stw_letter_count(char *buff, int position, char c);
int		stw_word_count(char *buff, char c);
char		*stw_str_n_cpy(char *str, int position, int len);
char		*get_extension(char *filename);

/*		capture			*/

int		capture(t_cam *c, char *root_path);
int		trigger_capture(t_cam *c, char **param);

/*		liveview		*/

int		liveviewfps(t_cam *c, char **param);
int		get_liveviewfps(t_cam *c, char **p);
int		liveview(t_cam *c, char **param);
void		*liveview_launcher(void *a);

/*		set_get_localconfig	*/

int		set_default_folder_path(t_cam *c, char **param);
int		get_default_folder_path(t_cam *c, char **param);

/*		set_get_remoteconfig	*/

int		set_config(char *key, char *val, t_cam *c);
int		set_get_config_init(char *key, CameraWidget **widget,
				    CameraWidget **child, t_cam *c);
int		get_config(char *key, t_cam *c);
int		get_all_widget_and_choices(t_cam *c);
char		*get_widget_choices(t_cam *c, CameraWidget *widget);
char		*get_widget_children(t_cam *c, CameraWidget *widget, char *info);
int		lookup_widget(CameraWidget*widget, const char *key, CameraWidget **child);

#ifdef __APPLE__

void		*initUSBDetect(void *main_struct);
int		usbDetect();

#endif

#endif /* !__CAMERA_CONTROL_H__ */
