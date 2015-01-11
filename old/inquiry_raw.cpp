// http://www.xgarreau.org/aide/divers/bluetooth/devel.php 
#include <netdb.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
 
int main (int argc, char **argv)
{
	int sock, retval;
	int i;
	unsigned char buf[HCI_MAX_FRAME_SIZE];
	struct sockaddr_hci addr;
	struct hci_filter filter;
	unsigned char cmd[] = {0x01, 0x01, 0x04, 0x05, 0x33, 0x8B, 0x9E, 0x08, 0x0A};
	int cmd_len = sizeof(cmd);
	int encore = 1;
 
	sock = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
	if (-1 == sock) exit(1);
 
	hci_filter_clear(&filter);
	hci_filter_all_ptypes(&filter);
	hci_filter_all_events(&filter);
 
	retval = setsockopt(sock, SOL_HCI, HCI_FILTER, &filter, sizeof(filter)); 
	if (-1 == retval) exit(1);
 
	addr.hci_family = AF_BLUETOOTH;
	addr.hci_dev = 0;
	retval = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
	if (-1 == retval) exit(1);
 
	retval = send (sock, cmd, cmd_len, 0);
	if (-1 == retval) exit(1);
 
	do {
		memset (buf, 0, sizeof(buf));
		retval = recv (sock, buf, sizeof(buf), 0);
		if (-1 == retval) exit(1);
		switch (buf[1]) {
			case EVT_CMD_STATUS:
				if (buf[3]) {
					printf ("Erreur !\n");
					encore = 0;
				} else {
					printf ("Commande en cours\n");
				}
				break;
			case EVT_INQUIRY_RESULT: 
				printf ("Périphérique trouvé:\n");
				printf ("  * Adresse : %02x:%02x:%02x:%02x:%02x:%02x\n",
					buf[9], buf[8],
					buf[7], buf[6],
					buf[5], buf[4]);
				printf ("  * Classe : 0x%02x%02x%02x\n",
					buf[15], buf[14], buf[13]);
				break;
			case EVT_INQUIRY_COMPLETE:
				encore = 0;
				break;
			default:
				break;
		}
	} while (encore);
 
	close (sock);
 
	return 0;
}
