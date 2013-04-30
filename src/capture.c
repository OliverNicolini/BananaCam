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

int		capture(t_cam *c, char *root_path)
{
  int		fd;
  int		retval;
  CameraFile	*file;
  CameraFilePath camera_file_path;
  int		waittime = 2000;
  CameraEventType type;
  void		*data;
  char		*message;
  char		*full_path;
  char		*tmp;

  retval = gp_camera_capture(c->camera, GP_CAPTURE_IMAGE, &camera_file_path, c->context);

  if (retval < GP_OK) {
    creat_and_send_message(KO, NULL, NULL, "camera capture failed", c);
    return (GP_ERROR);
  }

  asprintf(&full_path, "%s%s", root_path, camera_file_path.name);

  asprintf(&tmp, "Capturing to file %s\n", full_path);
  creat_and_send_message(INFO, NULL, NULL, tmp, c);
  free(tmp);

  fd = open(full_path, O_CREAT | O_WRONLY, 0644);

  retval = gp_file_new_from_fd(&file, fd);

  if (retval < GP_OK) {
    asprintf(&message, "%s creation failed", full_path);
    creat_and_send_message(KO, NULL, NULL, message, c);
    free(message);
    free(full_path);
    return (GP_ERROR);
  }

  free(full_path);

  retval = gp_camera_file_get(c->camera, camera_file_path.folder, camera_file_path.name,
			      GP_FILE_TYPE_NORMAL, file, c->context);

  if (retval < GP_OK) {
    asprintf(&message, "%s/%s retriving from camera failed", camera_file_path.folder,
	     camera_file_path.name);
    creat_and_send_message(KO, NULL, NULL, message, c);
    free(message);
    return (GP_ERROR);
  }

  retval = gp_camera_file_delete(c->camera, camera_file_path.folder, camera_file_path.name,
				 c->context);
  if (retval < GP_OK) {
    asprintf(&message, "%s/%s deleting into camera failed", camera_file_path.folder,
	     camera_file_path.name);
    creat_and_send_message(KO, NULL, NULL, message, c);
    free(message);
    return (GP_ERROR);
  }

  gp_file_free(file);

  while(1) {
    retval = gp_camera_wait_for_event(c->camera, waittime, &type, &data, c->context);

    if(type == GP_EVENT_TIMEOUT) {
      break;
    }
    else if (type == GP_EVENT_FILE_ADDED)
      {
	camera_file_path = *(CameraFilePath*)data;

	asprintf(&full_path, "%s%s", root_path, camera_file_path.name);

	asprintf(&tmp, "Capturing to file %s\n", full_path);
	creat_and_send_message(INFO, NULL, NULL, tmp, c);
	free(tmp);

	fd = open(full_path, O_CREAT | O_WRONLY, 0644);

	retval = gp_file_new_from_fd(&file, fd);

	if (retval < GP_OK) {
	  asprintf(&message, "%s creation failed", full_path);
	  creat_and_send_message(KO, NULL, NULL, message, c);
	  free(message);
	  free(full_path);
	  return (GP_ERROR);
	}
	retval = gp_camera_file_get(c->camera, camera_file_path.folder, camera_file_path.name,
				    GP_FILE_TYPE_NORMAL, file, c->context);

	retval = gp_camera_file_delete(c->camera, camera_file_path.folder, camera_file_path.name,
				       c->context);
	if (retval < GP_OK) {
	  asprintf(&message, "%s/%s deleting into camera failed", camera_file_path.folder,
		   camera_file_path.name);
	  creat_and_send_message(KO, NULL, NULL, message, c);
	  free(message);
	  return (GP_ERROR);
	}

	gp_file_free(file);
      }
    else if (type == GP_EVENT_CAPTURE_COMPLETE) {
      waittime = 100;
      break;
    }
    else if (type != GP_EVENT_UNKNOWN) {
      printf("Unexpected event received from camera: %d\n", (int)type);
    }
  }
  return (GP_OK);
}

int		trigger_capture(t_cam *c, char **param)
{
  int		nShots = 1;
  int		i = 0;
  char		*folder_path;

  if (param)
    {
      if (param[0])
	nShots = atoi(param[0]);
      if (param[1])
	folder_path = param[1];
      else
	folder_path = c->folder_path;
    }
  else
    folder_path = c->folder_path;

  creat_and_send_message(WAIT_RESPONSE, NULL, NULL, "Waiting capture complete", c);
  while (i < nShots)
    {
      capture(c, folder_path);
      i++;
    }
  creat_and_send_message(COMPLETE, NULL, NULL, "capture done", c);
  return (GP_OK);
}
