/**
 *
 * MOP™ v.1 by Student X
 * ------------------------------------------------------------------------------
 *     __  ___      ____                 ____             __                   __
 *    /  |/  /_  __/ __ \_      ______  / __ \_________  / /_____  _________  / /
 *   / /|_/ / / / / / / / | /| / / __ \/ /_/ / ___/ __ \/ __/ __ \/ ___/ __ \/ /
 *  / /  / / /_/ / /_/ /| |/ |/ / / / / ____/ /  / /_/ / /_/ /_/ / /__/ /_/ / /
 * /_/  /_/\__, /\____/ |__/|__/_/ /_/_/   /_/   \____/\__/\____/\___/\____/_/
 *        /____/
 * ------------------------------------------------------------------------------
 *
 * MOP™ (My Own Protocol) is a very simple messaging protocol over TCP
 * It uses a 8 byte header to communicate message type and payload size
 * All messages are acknowledged (ACK) when received, including header transfers
 * This allows both server and client to know that the connection is up and running
 *
 * Message types:
 * CONNECT - Client to server message on a new connection. MUST be first message in the connection
 *  * Server responds with ACCEPT or DECLINE
 *  * Message should contain no other data (DH.Bytes == 0)
 *
 * ACCEPT - Server response to Client on CONNECT request on successful connection
 *  * Message should contain the servers ID
 *  * DH MUST contain the size of the server ID (DH.Bytes > 0)
 *
 * DECLINE - Server response to Client on CONNECT request on failed connection
 *  * Message should contain the reason for the failed connection
 *  * DH MUST contain the size of the message (DH.Bytes > 0)
 *
 * MESSAGE - Used to send a payload to the peer
 *  * Message should contain a string that the server should print
 *  * DH MUST contain the size of the message (DH.Bytes > 0)
 *
 * ACK - Used to acknowledge any received message. Verifies two-way connection
 *  * Message should contain no other data (DH.Bytes == 0)
 *
 */

#ifndef OPPGAVE_7_MOP_H
#define OPPGAVE_7_MOP_H

#include <sys/socket.h>
#include <signal.h>
#include <string.h>

typedef struct _DATAHEADER {
    int Type;
    int Bytes;
} DH;

#define BUFFER_SIZE 256

#define CONNECT 0
#define ACCEPT  1
#define DECLINE 2
#define MESSAGE 3
#define ACK     4

/**
 * Sends a message on the socket
 *
 * Returns 0 on success, 1 on internal error and 2 on socket/peer error
 */
int SendMOPMessage(int fdSocket, int MsgType, char *szMsg);

/**
 * Retrieves a message on the socket
 *
 * Returns 0 on success, 1 on internal error and 2 on socket/peer error
 */
int ReceiveMOPMessage(int fdSocket, DH *stHeader, char *szBuffer, int iFlags);

#endif //OPPGAVE_7_MOP_H
