#include <netdb.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <stdlib.h>
#include <unistd.h>

int main (int argc, char **argv) {
  int dev_id, num_rsp, length, flags;
  inquiry_info *info = NULL;
  bdaddr_t bdaddr;
  int i;
  dev_id = 1; /* device hci 0 */
  length = 8; /* 10.24 seconds */
  num_rsp = 10;
  flags = 0 ;
  num_rsp = hci_inquiry (dev_id, length, num_rsp, NULL, &info, flags);
  for (i=0 ; i<num_rsp ; i++ ) {
      baswap (&bdaddr, &(info+i)->bdaddr);
      printf ("\t%s\n", batostr(&bdaddr)) ;
    }
  free (info);
  return 0;
} 
