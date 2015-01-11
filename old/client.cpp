// http://forum.ubuntu-it.org/viewtopic.php?p=3336279&mobile=off
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

int main(int argc, char **argv)
{
  struct sockaddr_rc addr = { 0 };
  int s, status;
  char dest[18] = "D0:39:72:B7:AF:66"; // '00:1F:81:00:01:1C";
  dest[18]='\0';

  // allocate a socket
  //s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
  s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
  printf("%d\n",s);
  // set the connection parameters (who to connect to)
  addr.rc_family = AF_BLUETOOTH;
  addr.rc_channel = (uint8_t) 1;
  str2ba( dest, &addr.rc_bdaddr );

  // connect to server
  status = connect(s, (struct sockaddr *)&addr, sizeof(addr));

  // send a message
  if( status == 0 ) {
    status = send(s, "hello!", 6,0);
  }

  if( status < 0 ) perror("uh oh");

  close(s);
  return 0;
}
