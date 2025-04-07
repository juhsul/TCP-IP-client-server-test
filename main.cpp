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
}

void client(const int send_port) {
    mtx.lock();
    cout << "Send port: " << send_port << endl;
    mtx.unlock();
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
