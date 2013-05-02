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

void		*linux_monitor_usb(void *obj)
{
  struct udev		*udev;
  struct udev_device	*dev;
  struct udev_monitor	*mon;
  int			fd;
  fd_set		fds;
  int			ret;

  t_cam *c = (t_cam*)obj;

  udev = udev_new();
  if (!udev)
    {
      printf("Can't create udev\n");
      return (NULL);
    }

  mon = udev_monitor_new_from_netlink(udev, "udev");
  udev_monitor_enable_receiving(mon);
  fd = udev_monitor_get_fd(mon);

  while (42)
    {
      FD_ZERO(&fds);
      FD_SET(fd, &fds);
      ret = select(fd+1, &fds, NULL, NULL, NULL);

      if (ret > 0 && FD_ISSET(fd, &fds))
	{
	  dev = udev_monitor_receive_device(mon);
	  if (dev)
	    {
	      if (strcmp(udev_device_get_devtype(dev), "usb_device") == 0)
		{
		  if (strcmp("add", udev_device_get_action(dev)) == 0)
		    {
		      if (!c->cam_usb)
			c->cam_usb = malloc(sizeof(*c->cam_usb));
		      c->cam_usb->vendor_id = (char *)strdup(udev_device_get_sysattr_value(dev,"idVendor"));
		      c->cam_usb->product_id = (char *)strdup(udev_device_get_sysattr_value(dev, "idProduct"));
		      c->cam_usb->product = (char *)strdup(udev_device_get_sysattr_value(dev,"product"));
		      c->cam_usb->vendor = (char *)strdup(udev_device_get_sysattr_value(dev,"manufacturer"));
		      c->cam_usb->node = (char *)strdup(udev_device_get_devnode(dev));
		      c->cam_usb->pluggued = 1;
		      printf("Added device %s\n", c->cam_usb->product);
		      c->init_cam(c);
		      get_all_widget_and_choices(c);
		      write(c->sock_struct->first_client->sock, c->camera_value_list, strlen(c->camera_value_list));
		    }
		  else if (strcmp("remove", udev_device_get_action(dev)) == 0)
		    {
		      if (c != NULL && c->cam_usb != NULL && c->cam_usb->pluggued == 1 && c->cam_usb->node && strcmp(udev_device_get_devnode(dev), c->cam_usb->node) == 0)
			{
			  printf("Removed device %s\n", c->cam_usb->product);
			  c->cam_usb->pluggued = 0;
			  gp_camera_exit(c->camera, c->context);
			}
		    }
		  udev_device_unref(dev);
		}
	      else if (strcmp(udev_device_get_devtype(dev), "usb_interface") != 0)
		printf("No Device from receive_device(). An error occured.\n");
	    }
	}
    }
  udev_unref(udev);
  return 0;
}

