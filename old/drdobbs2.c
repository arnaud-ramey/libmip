//#include "btinclude.h"
#include <netdb.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <stdlib.h>
#include <unistd.h>

uint8_t channel = 3;

int main()
{

  int sock;       // socket descriptor for local listener
  int client; // socket descriptor for remote client
  unsigned int len = sizeof(struct sockaddr_rc);

  struct sockaddr_rc remote;      // local rfcomm socket address
  struct sockaddr_rc local;       // remote rfcomm socket address
  char pszremote[18];

  // initialize a bluetooth socket
  sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

  local.rc_family = AF_BLUETOOTH;

  // TODO: change this to a local address if you know what
  // address to use
  local.rc_bdaddr = *BDADDR_ANY;
  local.rc_channel = channel;

  // bind the socket to a bluetooth device
  if (bind(sock, (struct sockaddr *)&local,
           sizeof(struct sockaddr_rc)) < 0)
    return -1;

  // set the listening queue length
  if (listen(sock, 1) < 0)
    return -1;

  printf("accepting connections on channel: %d\n", channel);

  // accept incoming connections; this is a blocking call
  client = accept(sock, (struct sockaddr *)&remote, &len);

  ba2str(&remote.rc_bdaddr, pszremote);

  printf("received connection from: %s\n", pszremote);

  return 0;
}
