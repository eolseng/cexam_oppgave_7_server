#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "./include/mop.h"
#include "./include/threadmanager.h"
#include "./include/simplelogger.h"

typedef struct _CONNECTION {
    int fdSocket;
    char *szServerName;
} CONNECTION;


int VerifyAndSetArgs(int iArgC, char **apszArgV, int *iPort, char **pszServerName);

void *HandleConnection(CONNECTION *conn);

int VerifyConnection(CONNECTION *conn);

void RejectConnection(CONNECTION *conn);

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

int main(int iArgC, char *apszArgV[]) {
    LogInfo("Starting 'Network Server' program");
    LogInfo("Log level set to %d", LOG_LEVEL);
    int iStatus = 0;

    int64_t ThreadManager;

    int iPort;
    char *szServerName;
    int fdServer, fdConnection;
    struct sockaddr_in saServerAddr = {0};
    int iAdrLen = sizeof(saServerAddr);

    if (VerifyAndSetArgs(iArgC, apszArgV, &iPort, &szServerName) != 0) {
        LogError("Failed to verify args. Terminating program.");
        iStatus = 1;
    } else {
        if (CreateManager(&ThreadManager) != 0) {
            LogError("Failed to create Thread Manager");
            iStatus = 1;
        } else {
            // Create a socket
            fdServer = socket(AF_INET, SOCK_STREAM, 0);
            if (fdServer < 0) {
                LogError("Failed to open socket");
                iStatus = 1;
            } else {
                // Setup socket struct
                saServerAddr.sin_family = AF_INET;
                saServerAddr.sin_port = htons(iPort);
                saServerAddr.sin_addr.s_addr = INADDR_ANY;
                // Bind socket
                if (bind(fdServer, (struct sockaddr *) &saServerAddr, sizeof(saServerAddr)) < 0) {
                    LogError("Failed to bind socket");
                    iStatus = 1;
                } else {
                    if (listen(fdServer, 5) < 0) {
                        LogError("Failed to listen");
                        iStatus = 1;
                    } else {
                        LogInfo("Socket up and running. Listening on port %d", iPort);
                        printf("Server listening on port %d. Exit with 'ctrl-c", iPort);
                        while (1) {
                            fdConnection = accept(fdServer, (struct sockaddr *) NULL, (socklen_t *) &iAdrLen);
                            if (fdConnection < 0) {
                                LogError("Failed to accept connection");
                                iStatus = 1;
                                break;
                            } else {
                                LogInfo("New connection established");
                                // Setup connection args - cleanup is done in Handle/RejectConnection
                                CONNECTION *conn = malloc(sizeof(CONNECTION));
                                if (conn == NULL) {
                                    LogError("Failed to allocate memory for CONNECTION");
                                    iStatus = 1;
                                    break;
                                } else {
                                    LogDebug("MALLOC CONN ADR.: %u", conn); // Used to verify no memory leak
                                    conn->fdSocket = fdConnection;
                                    conn->szServerName = szServerName;
                                    // Handle the connection
                                    if (ExecuteFunction(ThreadManager, HandleConnection, (void *) conn) != 0) {
                                        LogWarn("All threads busy - could not handle connection");
                                        RejectConnection(conn);
                                    }
                                }
                            }
                        }
                    }
                }
                close(fdServer);
            }
            DestroyManager(ThreadManager);
        }
    }

    if (iStatus != 0)
        LogInfo("Program failed with status code %d", iStatus);
    else
        LogInfo("Program finished successfully");
    return iStatus;
}

#pragma clang diagnostic pop

void RejectConnection(CONNECTION *conn) {
    DH stHeader = {0};
    ReceiveMOPMessage(conn->fdSocket, &stHeader, NULL, MSG_DONTWAIT);
    SendMOPMessage(conn->fdSocket, DECLINE, "Server is busy. Please try again later");
    LogDebug("FREE CONN ADR.: %u", conn); // Used to verify no memory leak
    free(conn);
    conn = NULL;
}

int VerifyAndSetArgs(int iArgC, char **apszArgV, int *iPort, char **pszServerName) {

    /*
     * Verifies that four arguments is supplied and sets the values for port and server name
     * First finds "-port" and then tries to convert the next argument into a long, casted to an int
     * Then finds "-id" and sets the server address to the next argument
     *
     * Does not verify that the argument following "-port" and "-id".
     * "-id" could be followed by "-port" etc.
     *
     * Works as long as program is used as intended.
     */

    int i, iStatus = 0;

    if (iArgC != 5) {
        LogError("Wrong amount of arguments supplied. Expected 4, got %d", iArgC - 1);
        printf("ERROR: Wrong amount of arguments provided.\r\n");
        printf("ERROR: Please provide arguments: '-port [PORT_NUMBER]' and '-id [SERVER_NAME]'\r\n");
        iStatus = 1;
    } else {

        // Validate -port argument
        int bFoundPort = 0;
        for (i = 1; i < iArgC; i++) {
            if (strcmp(apszArgV[i], "-port") == 0) {
                bFoundPort = 1;
                *iPort = (int) strtol(apszArgV[i + 1], NULL, 10);
                if (*iPort == 0) {
                    LogError("Failed to convert port to integer. Port argument: '%s'", apszArgV[i + 1]);
                    printf("ERROR: Failed to convert port to number. Provided: '%s'\r\n", apszArgV[i + 1]);
                    iStatus = 1;
                }
                break;
            }
        }
        if (!bFoundPort) {
            printf("ERROR: Please supply '-port [PORT_NUMBER]' argument to program");
            iStatus = 1;
        }

        // Validate -id argument
        int bFoundId = 0;
        for (i = 1; i < iArgC; i++) {
            if (strcmp(apszArgV[i], "-id") == 0) {
                bFoundId = 1;
                *pszServerName = apszArgV[i + 1];
                break;
            }
        }
        if (!bFoundId) {
            printf("ERROR: Please supply '-id [SERVER_NAME]' argument to program");
            iStatus = 1;
        }
    }

    return iStatus;

}

void *HandleConnection(CONNECTION *conn) {

    int fdSocket = conn->fdSocket;
    char szBuffer[BUFFER_SIZE] = {0};
    DH stHeader = {0};
    int iReceived, bActive = 1;

    if (VerifyConnection(conn) != 0) {
        LogError("Client failed to connect");
    } else {
        LogInfo("Client connected");
        while (bActive) {
            iReceived = ReceiveMOPMessage(fdSocket, &stHeader, szBuffer, MSG_WAITALL);
            if (iReceived == 1) {
                LogError("Failed to receive message.");
                bActive = 0;
            } else if (iReceived == 2) {
                // Client closed connection
                bActive = 0;
            } else {
                switch (stHeader.Type) {
                    case MESSAGE:
                        printf("MESSAGE: %s\r\n", szBuffer);
                        break;
                    default:
                        LogWarn("Invalid message type received on open connection. Type: %d", stHeader.Type);
                        break;
                }
            }
        }
    }
    if (close(fdSocket) != 0) {
        LogError("Failed to close connection");
    } else {
        LogInfo("Client disconnected");
    }

    // Cleanup the conn
    LogDebug("FREE CONN ADR.: %u", conn); // Used to verify no memory leak
    free(conn);
    conn = NULL;

    return 0;
}

int VerifyConnection(CONNECTION *conn) {
    DH stHeader = {0};
    int iRec = ReceiveMOPMessage(conn->fdSocket, &stHeader, NULL, MSG_DONTWAIT);
    if (stHeader.Type != CONNECT || iRec == 1) {
        SendMOPMessage(conn->fdSocket, DECLINE, "Client must connect with msg type CONNECT");
        return 1;
    } else if (iRec == 2) {
        LogInfo("Peer closed connection");
        return 2;
    } else {
        SendMOPMessage(conn->fdSocket, ACCEPT, conn->szServerName);
        return 0;
    }
}
