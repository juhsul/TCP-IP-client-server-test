#include <iostream>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

void server(const int listen_port) {

    // Luodaan socket (TCP)
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET) {
        cerr << "Socket creation failed: " << WSAGetLastError() << "\n";
        return;
    }

    // Osoitetiedot
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;                     // IPv4
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Kuuntele paikallisesti
    server_addr.sin_port = htons(listen_port);            // Portti

    // Bindaa socket osoitteeseen ja porttiin
    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        cerr << "Bind failed: " << WSAGetLastError() << "\n";
        closesocket(server_socket);
        return;
    }

    // Kuuntele saapuvia yhteyksiä
    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "Listen failed: " << WSAGetLastError() << "\n";
        closesocket(server_socket);
        return;
    }

    cout << "Listening on port " << ntohs(server_addr.sin_port) << "\n";

    sockaddr_in client_addr;
    int client_size = sizeof(client_addr);

    // Hyväksy yksi yhteys
    SOCKET client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_size);
    if (client_socket == INVALID_SOCKET) {
        cerr << "Accept failed: " << WSAGetLastError() << "\n";
    } else {
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        int client_port = ntohs(client_addr.sin_port);

        cout << "Connection accepted from " << client_ip << ":" << client_port << "\n";
    }

    while (true) {
        // Viestin vastaanotto
        char buffer[1024]; // Puskuri vastaanotettavalle datalle
        int bytesReceived = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0'; // Lisätään lopetusmerkki stringiksi
            cout << "Client sent: " << buffer << std::endl;

        } else if (bytesReceived == 0) {
            cout << "Connection closed by client.\n";
            break;
        } else {
            cerr << "Receiving failed: " << WSAGetLastError() << std::endl;
        }
    }

    // Sulje socketit ja vapauta Winsock
    closesocket(client_socket);
    closesocket(server_socket);
}

void client(const int send_port) {

    // Luodaan socket (TCP)
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << "\n";
        return;
    }

    // Osoitteet
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;                     // IPv4
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Palvelimen IP-osoite
    server_addr.sin_port = htons(send_port);              // Palvelimen kuunteluportti

    while (true) {
        // Yhteyden muodostaminen
        if (connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
            cerr << "Connection failed: " << WSAGetLastError() << "\n";
            cout << "Trying again in 5 seconds." << endl;
            Sleep(5000);
        } else {
            cout << "Connected!" << endl;
            break;
        }
    }

    string message;
    while (true) {

        cout << "Message to be sent: ";
        getline(cin, message);
        if (message == "-1") {
            break;
        }

        int bytesSent = send(client_socket, message.c_str(), message.length(), 0);
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Send failed: " << WSAGetLastError() << "\n";
        }
    }

    closesocket(client_socket);
}

int main(int argc, char* argv[]) {
    string input;

    // Windows Sockets alustus
    WSADATA wsaData;
    int wsaerr = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaerr != 0) {
        cerr << "WSAStartup failed.\n";
        return 1;
    }

    // Käynnistetään vain toinen prosessi riippuen siitä ollaanko palvelin, vai asiakas
    if (argc > 1 && strcmp(argv[1], "server") == 0) {
        cout << "Starting server" << endl;

        cout << "Listen port: ";
        getline(cin, input);
        int listen_port = stoi(input);

        server(listen_port);
    } else {
        cout << "Starting client" << endl;

        cout << "Send port: ";
        getline(cin, input);
        int send_port = stoi(input);

        client(send_port);
    }

    WSACleanup();
    return 0;
}
