/*
COPYRIGHT:          Copyleft (C) 2023

ORIGINAL AUTHOR(S): https://github.com/mydarkthawts

LICENSE:            Open Source, just give credit where it is due and pay it forward.

DESCRIPTION:        Spins up an http web server and opens a console window and a
                    webpage pointed at localhost:666 in the systems default web
                    browser to show the built in html dashboard with options to
                    select and a submit button to submit the choices to the web
                    server.
                    The console displays actions made in the webpage once the
                    Submit button is pressed. The web page will also reflect the
                    actions chosen.

NOTES:              Simply a proof of concept code base template that can be expanded
                    upon later for numerous project types. This code does nothing but
                    display the actions/choices submitted in text form.
*/

#include <iostream>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <shellapi.h>

#pragma comment(lib, "ws2_32.lib")

const char* HTML_CONTENT = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Http Server Dashboard</title>
</head>
<body>
    <h1>Welcome to my ugly sample http server dashboard.</h1>

    <h2>Select an option</h2>
    <select id="dropdown">
    <option value = "option 1">Chocolate</option>
    <option value = "option 2">Chicken</option>
    <option value = "option 3">Beer</option>
    </select>
    <br>
    <div id = "selectedOption"></div>

    <button onclick="performAction() ">TOUCH ME!</button>

<script>
function performAction() 
{
    var dropdown = document.getElementById("dropdown");
    var selectedOption = dropdown.options[dropdown.selectedIndex].text;

    var selectedOptionDiv = document.getElementById("selectedOption");
    selectedOptionDiv.innerHTML = "Selected option: " + selectedOption;

    // Perform a POST request to the server
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "/action", true);
    xhr.setRequestHeader("Content-Type", "text/plain");
    xhr.onreadystatechange = function() 
    {
        if (xhr.readyState == XMLHttpRequest.DONE && xhr.status == 200) 
        {
            console.log("Server response: " + xhr.responseText);
        }
    };
    xhr.send(selectedOption);
}
</script>
</body>
</html>
)";

std::string processPostData(const std::string & postData) {
    // The postData contains the selected options from the dropdown
    // Here, we can check the value of postData and perform the desired action
    if (postData == "Chocolate")
    {
        // Perform action for Option 1
        std::cout << "Chocolate option selected!\n";
        return "Chocolate option selected!";
    }
    else if (postData == "Chicken")
    {
        // Perform action for Option 2
        std::cout << "Chicken option selected!\n";
        return "Chicken option selected!";
    }
    else if (postData == "Beer")
    {
        // Perform action for Option 3
        std::cout << "Beer option selected!\n";
        return "Beer option selected!";
    }
    else
    {
        std::cout << "Invalid option!\n";
        return "Invalid option!";
    }
}

void SendResponse(SOCKET clientSocket, const std::string & response)
{
    std::string httpResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(response.length()) + "\r\nConnection: close\r\n\r\n" + response;

    send(clientSocket, httpResponse.c_str(), httpResponse.length(), 0);
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Failed to initialize Winsock\n";
        return 1;
    }

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET)
    {
        std::cerr << "Failed to create socket\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(666); // Change 666 to the desired port number

    if (bind(listenSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        std::cerr << "Failed to bind socket\n";
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "Listen failed\n";
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Http web server with dashboard\n";
    std::cout << "Server listening on port 666\n";
    ShellExecute(0, 0, L"http://localhost:666", 0, 0, SW_SHOW);

    while (true)
    {
        sockaddr_in clientAddress;
        int clientAddressSize = sizeof(clientAddress);

        SOCKET clientSocket = accept(listenSocket, (struct sockaddr*)&clientAddress, &clientAddressSize);
        if (clientSocket == INVALID_SOCKET)
        {
            std::cerr << "Accept failed\n";
            closesocket(listenSocket);
            WSACleanup();
            return 1;
        }

        // Process the incoming HTTP request
        char buffer[1024];
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0)
        {
            buffer[bytesReceived] = '\0';
            std::string request = buffer;

            if (request.find("POST /action") != std::string::npos)
            {
                // Extract the POST data
                std::string postData = request.substr(request.find("\r\n\r\n") + 4);

                // Process the data and generate the processed response
                std::string response = processPostData(postData);

                // Send the response back to the client
                SendResponse(clientSocket, response);
            }
            else
            {
                // Send the HTML dashboard code as the default response
                SendResponse(clientSocket, HTML_CONTENT);
            }
        }

        closesocket(clientSocket);
    }

    closesocket(listenSocket);
    WSACleanup();
    return 0;
}