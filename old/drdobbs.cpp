//http://www.drdobbs.com/mobile/using-bluetooth/232500828
//#include "btinclude.h"
#include <netdb.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
  int id;
  int fh;
  bdaddr_t btaddr;
  char pszaddr[18];

  unsigned int cls = 0x280404;
  int timeout = 1000;

  printf("this example should be run as root\n");

  // get the device ID
  if ((id = hci_get_route(NULL)) < 0)
    return -1;

  // convert the device ID into a 6 byte bluetooth address
  if (hci_devba(id, &btaddr) < 0)
    return -1;

  // convert the address into a zero terminated string
  if (ba2str(&btaddr, pszaddr) < 0)
    return -1;

  // open a file handle to the HCI
  if ((fh = hci_open_dev(id)) < 0)
    return -1;

  // set the class
  if (hci_write_class_of_dev(fh, cls, timeout) != 0)
  {
    perror("hci_write_class ");
    return -1;
  }

  // close the file handle
  hci_close_dev(fh);

  printf("set device %s to class: 0x%06x\n", pszaddr, cls);

  return 0;
}
