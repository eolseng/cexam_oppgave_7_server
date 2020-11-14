#include "./include/mop.h"

int SendMOPMessage(int fdSocket, int MsgType, char *szMsg) {

    int iSent, iConf, iMsgSize = (int) strlen(szMsg);
    DH stHeader = {0}, stConfHeader = {0};
    stHeader.Type = MsgType;
    stHeader.Bytes = iMsgSize;

    // Ignore SIGPIPE errors from socket disconnect
    signal(SIGPIPE, SIG_IGN);

    // Send MOP Header to peer
    iSent = send(fdSocket, &stHeader, sizeof(DH), 0);
    if (iSent != sizeof(DH)) return 1;
    // Confirm OK with peer
    iConf = recv(fdSocket, &stConfHeader, sizeof(DH), MSG_WAITALL);
    if (iConf != sizeof(DH) || stConfHeader.Type != ACK || stConfHeader.Bytes != iSent) return 2;

    if (iMsgSize > 0) {
        // Reset the Confirm Header
        memset(&stConfHeader, 0, sizeof(DH));
        // Send data over socket
        iSent = send(fdSocket, szMsg, iMsgSize, 0);
        if (iSent != iMsgSize) return 1;
        // Confirm OK with peer
        iConf = recv(fdSocket, &stConfHeader, sizeof(DH), MSG_WAITALL);
        if (iConf != sizeof(DH) || stConfHeader.Type != ACK || stConfHeader.Bytes != iSent) return 2;
    }
    return 0;
}

int ReceiveMOPMessage(int fdSocket, DH *stHeader, char *szBuffer, int iFlags) {

    int iRec, iConf;
    DH stConfHeader = {0};
    stConfHeader.Type = ACK;
    stConfHeader.Bytes = 0;

    // Reset header and buffer
    memset(stHeader, 0, sizeof(DH));
    if (szBuffer) bzero(szBuffer, BUFFER_SIZE);

    // Retrieve header and confirm
    iRec = recv(fdSocket, stHeader, sizeof(DH), iFlags);
    if (iRec < 0) return 1; // Error reading from socket
    else if (iRec == 0) return 2; // Socket is closed
    // Confirm received with peer
    stConfHeader.Bytes = iRec; // Confirm bytes received
    iConf = send(fdSocket, &stConfHeader, sizeof(DH), 0); // Send ACK-header with bytes received
    if (iConf != sizeof(DH)) return 1; // Failed to send confirm

    if (stHeader->Bytes > 0) {
        // Retrieve data from socket
        iRec = recv(fdSocket, szBuffer, stHeader->Bytes, MSG_WAITALL);
        if (iRec < 0) return 1; // Error reading from socket
        else if (iRec == 0) return 2; // Socket is closed
        // Confirm received with peer
        stConfHeader.Bytes = iRec; // Confirm bytes received
        iConf = send(fdSocket, &stConfHeader, sizeof(DH), 0); // Send ACK-header with bytes received
        if (iConf != sizeof(DH)) return 1; // Failed to send confirm
    }
    return 0;
}
