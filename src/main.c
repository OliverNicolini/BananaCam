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

void error_func (GPContext *context, const char *format, va_list args, void *data) {
  context = context;
  data = data;
  fprintf(stderr, "*** Contexterror ***\n");
  vfprintf(stderr, format, args);
  fprintf(stderr, "\n");
}

void message_func (GPContext *context, const char *format, va_list args, void *data) {
  context = context;
  data = data;
  vprintf(format, args);
  printf("\n");
}

void			signal_handler(int sig)
{
  printf("Signal ==> %i\n", sig);
}

void			signal_inib()
{
  struct sigaction	act;
  int			i;

  act.sa_handler = signal_handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  i = 1;
  while (i < 32)
    {
      if (i != 11)
	sigaction(i, &act, NULL);
      i++;
    }
}

int		init(t_cam *c)
{
  c->liveview = 0;
  c->liveview_fps = 30;
  c->liveview_fps_time = 1000000 / 30;
  pthread_mutex_init(&c->liveview_mutex, NULL);
  pthread_cond_init(&c->liveview_condvar, NULL);
  c->folder_path = strdup("/tmp/");
  c->camera_value_list = NULL;

  gp_context_set_error_func(c->context, (GPContextErrorFunc)error_func, NULL);
  gp_context_set_message_func(c->context, (GPContextMessageFunc)message_func, NULL);

  gp_camera_new(&c->camera);
  c->context = gp_context_new();
  printf("Camera Init\n");
  c->ret = gp_camera_init(c->camera, c->context);
  if (c->ret != GP_OK) {
    printf("gp_camera_init: %d\n", c->ret);
    return (GP_ERROR);
  }
  /*  get_initial_camera_values(t_cam *c); */
  return (GP_OK);
}

void		generic_exec(t_cam *c, char *command, char **param)
{
  char		*msg = NULL;

  if (command && strncmp(command, "get_", 4) == 0)
    {
      command = &command[4];
      get_config(command, c);
      return;
    }
  if (param)
    {
      if (param[0])
	set_config(command, param[0], c);
    }
  else
    {
      asprintf(&msg, "bad parameters for %s", command);
      creat_and_send_message(BAD_PARAMETERS, NULL, NULL, msg, c);
    }
}

int		exec_command(t_cam *c, char *command, char **param)
{
  t_func	*tmp = NULL;
  int		flag = 0;

  if (strcmp(command, "liveview") != 0 && c->liveview == 1)
    {
      printf("enter inside here\n");
      c->liveview = 0;
      flag = 1;
      sleep(1);
    }
  tmp = c->first_func_ptr;
  while (tmp != NULL)
    {
      if (strcmp(command, tmp->name) == 0)
	{
	  tmp->func_ptr(c, param);
	  break;
	}
      tmp = tmp->next;
    }
  if (tmp == NULL)
    generic_exec(c, command, param);
  if (flag == 1)
    liveview(c, NULL);
  return (0);
}

void		add_func_ptr_list(t_cam *c, char *name, int (*func_ptr)(t_cam *c, char **param))
{
  t_func	*tmp;

  if (c->first_func_ptr == NULL)
    {
      c->first_func_ptr = malloc(sizeof(*c->first_func_ptr));
      tmp = c->first_func_ptr;
      tmp->next = NULL;
      tmp->func_ptr = func_ptr;
      tmp->name = strdup(name);
      c->first_func_ptr = tmp;
    }
  else
    {
      tmp = c->first_func_ptr;
      while (tmp->next != NULL)
	tmp = tmp->next;
      tmp->next = malloc(sizeof(*tmp->next));
      tmp = tmp->next;
      tmp->next = NULL;
      tmp->func_ptr = func_ptr;
      tmp->name = strdup(name);
    }
}

int		main(int ac, char **av)
{
  t_cam		*c;

  ac = ac;
  av = av;

  #ifdef __APPLE__

  //pthread_t	thread;

  printf("Killing PTPCamera process\n");
  system("killall PTPCamera");
  //pthread_create(&thread, NULL, initUSBDetect, (void *)c);

  #endif

  signal_inib();

  c = malloc(sizeof(*c));
  c->first_func_ptr = NULL;
  init(c);
  get_all_widget_and_choices(c);
  add_func_ptr_list(c, "capture", trigger_capture);
  add_func_ptr_list(c, "liveview", liveview);
  add_func_ptr_list(c, "auto_focus", auto_focus);
  add_func_ptr_list(c, "liveviewfps", liveviewfps);
  add_func_ptr_list(c, "get_liveviewfps", get_liveviewfps);
  add_func_ptr_list(c, "defaultpath", set_default_folder_path);
  add_func_ptr_list(c, "get_defaultpath", get_default_folder_path);

  pthread_create(&c->liveview_thread, NULL, liveview_launcher, (void*)c);

  init_comm(c, UNIX_SOCKET_PATH);
  gp_camera_exit(c->camera, c->context);
  return (0);
}







