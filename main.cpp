#include <iostream>
#include <mutex>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

mutex mtx; // Estää säikeiden samanaikaisen tulostuksen

void server(const int listen_port) {
    mtx.lock();
    cout << "Listen port: " << listen_port << endl;
    mtx.unlock();

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
        std::cerr << "Bind failed: " << WSAGetLastError() << "\n";
        closesocket(server_socket);
        return;
    }

    // Kuuntele saapuvia yhteyksiä
    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed: " << WSAGetLastError() << "\n";
        closesocket(server_socket);
        return;
    }

    std::cout << "Listening on port " << ntohs(server_addr.sin_port) << "\n";

    // Hyväksy yksi yhteys
    SOCKET client_socket = accept(server_socket, nullptr, nullptr);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "Accept failed: " << WSAGetLastError() << "\n";
    } else {
        std::cout << "Connection accepted!" << "\n";
    }

    // Viestin vastaanotto
    char buffer[1024]; // Puskuri vastaanotettavalle datalle
    int bytesReceived = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0'; // Lisätään lopetusmerkki stringiksi
        cout << "Client sent: " << buffer << std::endl;

        const char* msg = "Hello, client! I received your message!";
        int bytesSent = send(client_socket, msg, strlen(msg), 0);
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Send failed: " << WSAGetLastError() << "\n";
            closesocket(client_socket);
            return;
        }

    } else if (bytesReceived == 0) {
        cout << "Connection closed by client.\n";
    } else {
        cerr << "Receiving failed: " << WSAGetLastError() << std::endl;
    }

    // Sulje socketit ja vapauta Winsock
    closesocket(client_socket);
    closesocket(server_socket);
}

void client(const int send_port) {
    mtx.lock();
    cout << "Send port: " << send_port << endl;
    mtx.unlock();

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
            break;
        }
    }

    cout << "Message to be sent: ";
    string message;
    getline(cin, message);

    int bytesSent = send(client_socket, message.c_str(), message.length(), 0);
    if (bytesSent == SOCKET_ERROR) {
        std::cerr << "Send failed: " << WSAGetLastError() << "\n";
        closesocket(client_socket);
        return;
    }

    char buffer[512];
    int bytesReceived = recv(client_socket, buffer, sizeof(buffer), 0);

    if (bytesReceived == SOCKET_ERROR) {
        std::cerr << "Receive failed: " << WSAGetLastError() << "\n";
    } else {
        buffer[bytesReceived] = '\0';
        std::cout << "Received from server: " << buffer << std::endl;
    }

    closesocket(client_socket);
}

int main() {
    string input;
    int send_port;
    int listen_port;

    cout << "Send port: ";
    getline(cin, input);
    send_port = stoi(input);

    cout << "Listen port: ";
    getline(cin, input);
    listen_port = stoi(input);

    // Windows Sockets alustus
    WSADATA wsaData;
    int wsaerr = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaerr != 0) {
        cerr << "WSAStartup failed.\n";
        return 1;
    }

    // Käynnistetään kaksi säiettä että voidaan lähettää ja vastaanottaa samaan aikaan
    thread server_thread(server, listen_port);
    thread client_thread(client, send_port);

    server_thread.join();
    client_thread.join();

    WSACleanup();
    return 0;
}
