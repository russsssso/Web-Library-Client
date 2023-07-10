// Client side of a virtual library application
#include <iostream>
#include <string>
#include <arpa/inet.h>
#include "nlohmann/json.hpp"

extern "C" {
  #include "helpers.h"
}

using json = nlohmann::json;

using namespace std;

#define SERVER_IP "34.254.242.81"
#define SERVER_PORT 8080

/* Check if string is a valid input:
*  allowed characters: alphanumeric, underscore, dot, dash
*/
bool is_string_valid(string s) {
    for (unsigned long i = 0; i < s.length(); ++i) {
        if (!isalnum(s[i]) && s[i] != '_' && s[i] != '.' && s[i] != '-') {
            return false;
        }
    }
    return true;
}

/*  Check if string is a valid number:
*   allowed characters: digits
*/
bool is_number_valid(string s) {
    for (unsigned long i = 0; i < s.length(); ++i) {
        if (!isdigit(s[i])) {
            return false;
        }
    }
    return true;
}

/*  Post request to register a new user
*   Returns void, prints the response from the server
*/
void register_user() {
    // Prompt for username and password
    string username, password;
    cout << "username=";
    cin >> username;
    cout << "password=";
    cin >> password;

    if (!is_string_valid(username) || !is_string_valid(password)) {
        cout << "Error: Invalid username or password!" << endl;
        return;
    }
    
    int sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);

    // Create the json object
    json user;
    user["username"] = username;
    user["password"] = password;

    // Json to string
    string reg = user.dump();

    char *message = compute_post_request(SERVER_IP, "/api/v1/tema/auth/register", "application/json", reg.c_str(), reg.length(), NULL, 0, NULL);
    send_to_server(sockfd, message);
    free(message);
    
    message = receive_from_server(sockfd);
    string response = string(message);
    free(message);

    // Interpret response
    if (response.find("error") != string::npos) {
        cout << "Error: The username is taken!" << endl;
    } else {
        cout << "User " << username << " registered!" << endl;
    }
}

//  Extract cookie from string
string extract_cookie(string response) {
    int start = response.find("connect.sid");
    int end = response.find(";", start);
    return response.substr(start, end - start);
}

/*  Post request to login an existing user
*   Returns cookie, prints the response from the server
*/
string login() {
    // Prompt for username and password
    string username, password;
    cout << "username=";
    cin >> username;
    cout << "password=";
    cin >> password;

    if (!is_string_valid(username) || !is_string_valid(password)) {
        cout << "Error: Invalid username or password!" << endl;
        return string("-1");
    }

    int sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);

    // Create the json object
    json user;
    user["username"] = username;
    user["password"] = password;

    // Json to string
    string log = user.dump();

    char *message = compute_post_request(SERVER_IP, "/api/v1/tema/auth/login", "application/json", log.c_str(), log.length(), NULL, 0, NULL);
    send_to_server(sockfd, message);
    free(message);
    
    message = receive_from_server(sockfd);
    close_connection(sockfd);

    string response = string(message);
    free(message);

    // Interpret response
    if (response.find("error") != string::npos) {
        cout << "Error: Invalid Username or Password" << endl;
        return string("-1");
    } else {
        if (response.find("connect.sid") == string::npos) {
            cout << "Server did not respond, try again!" << endl;
            return string("-1");
        }

        cout << "User " << username << " loged in!" << endl;

        // Extract cookie
        return extract_cookie(response);
    }

}

/*  Extract JWT token from string
*/
string extract_token(string response) {
    int start = response.find("{\"token\"");
    json j = json::parse(response.substr(start));
    return j["token"];
}

/* Get request for library access
* Has as parameter the session cookie
* Prints the response from server and returns a JWT token if access is granted
*/
string enter_library(string cookie) {
    int sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);

    const char *cookies[1] = {cookie.c_str()}; 
    char *message = compute_get_request(SERVER_IP, "/api/v1/tema/library/access", NULL, cookies, 1, NULL);
    send_to_server(sockfd, message);
    free(message);

    message = receive_from_server(sockfd);
    string response = string(message);
    free(message);

    close_connection(sockfd);

    // Interpret response
    if (response.find("error") != string::npos) {
        cout << "Error: Invalid session" << endl;
        return string("-1");
    } else {
        if (response.find("token") == string::npos) {
            cout << "Server did not respond, try again!" << endl;
            return string("-1");
        }
        cout << "Library access granted!" << endl;
        
        // Extract JWT token
        return extract_token(response);
    }
}

/* Print all books in response
*/
void print_books(string response) {
    int start = response.find("[");
    int end = response.find("]");
    json j = json::parse(response.substr(start, end - start + 1));
    for (auto it = j.begin(); it != j.end(); ++it) {
        cout << "id=" << it.value()["id"];
        cout << "\ttitle=" << it.value()["title"] << endl;
    }
}

/*  Get request for all books in the library
*   Has as parameter the JWT token
*   Returns void, prints the response from the server
*/
void get_books(string token) {
    int sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);

    char *message = compute_get_request(SERVER_IP, "/api/v1/tema/library/books", NULL, NULL, 0, token.c_str());
    send_to_server(sockfd, message);
    free(message);

    message = receive_from_server(sockfd);
    string response = string(message);
    free(message);

    close_connection(sockfd);

    // Interpret response
    if (response.find("error") != string::npos) {
        cout << "Error: You don't have acces to the library!" << endl;
    } else {
        if (response.find("200 OK") == string::npos) {
            cout << "Server did not respond, try again!" << endl;
            return;
        } 
        print_books(response);
    }
}

/*  Print book
*/
void print_book(string response) {
    int start = response.find("{");
    int end = response.find("}");
    json j = json::parse(response.substr(start, end - start + 1));
    cout << "title=" << j["title"] << endl;
    cout << "author=" << j["author"] << endl;
    cout << "genre=" << j["genre"] << endl;
    cout << "page count=" << j["page_count"] << endl;
    cout << "publisher=" << j["publisher"] << endl;

}

/*  Get request for a book with a given id
*   Has as parameter the JWT token and a book id
*   Returns void, prints the response from the server
*/
void get_book(string token, string id) {

    if (!is_number_valid(id)) {
        cout << "Error: Invalid book id!" << endl;
        return;
    }

    int sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);

    string aux = string("/api/v1/tema/library/books/") + id;
    char *message = compute_get_request(SERVER_IP, aux.c_str(), NULL, NULL, 0, token.c_str());
    send_to_server(sockfd, message);
    free(message);

    message = receive_from_server(sockfd);
    string response = string(message);
    free(message);

    close_connection(sockfd);   

    // Interpret response
    if (response.find("\"error\":\"No book was found!\"") != string::npos) {
        cout << "Error: Book does not exist!" << endl;
    } else if (response.find("error") != string::npos) {
        cout << "Error: You don't have acces to the library!" << endl;
    } else {
        if (response.find("200 OK") == string::npos) {
            cout << "Server did not respond, try again!" << endl;
            return;
        } 
        print_book(response);
    }
}

/*  Post request to add a book to the library
*   Has as parameter the JWT token and book details: title, author, genre, page_count, publisher
*   Returns void, prints the response from the server
*/
void add_book(string token, string title, string author, string genre, string page_count, string publisher) {

    if (!is_string_valid(title) || !is_string_valid(author) || !is_string_valid(genre) 
        || !is_string_valid(publisher) || !is_number_valid(page_count)) {
        cout << "Error: Invalid book details!" << endl;
        return;
    }

    // Create the json object
    json book;
    book["title"] = title;
    book["author"] = author;
    book["genre"] = genre;
    book["page_count"] = page_count;
    book["publisher"] = publisher;

    // Json to string
    string add = book.dump();

    int sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);

    char *message = compute_post_request(SERVER_IP, "/api/v1/tema/library/books", "application/json", 
                                        add.c_str(), add.length(), NULL, 0, token.c_str());

    send_to_server(sockfd, message);
    free(message);
    
    message = receive_from_server(sockfd);
    string response = string(message);
    free(message);

    close_connection(sockfd);

    // Interpret response
    if (response.find("error") != string::npos) {
        cout << "Error: You don't have acces to the library!" << endl;
    } else {
        if (response.find("200 OK") == string::npos) {
            cout << "Server did not respond, try again!" << endl;
            return;
        } 
        cout << "Book added!" << endl;
    }
}

/* Delete request to delete a book from the library
*   Has as parameter the JWT token and a book id
*   Returns void, prints the response from the server
*/
void delete_book(string token, string id) {
    if (!is_number_valid(id)) {
        cout << "Error: Invalid book id!" << endl;
        return;
    }

    int sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);

    string aux = string("/api/v1/tema/library/books/") + id;
    char *message = compute_delete_request(SERVER_IP, aux.c_str(), NULL, NULL, 0, token.c_str());
    send_to_server(sockfd, message);
    free(message);

    message = receive_from_server(sockfd);
    string response = string(message);
    free(message);

    close_connection(sockfd);

    // Interpret response
    if (response.find("error") != string::npos) {
        cout << "Error: You don't have acces to the library!" << endl;
    } else if (response.find("404 Not Found") != string::npos) {
        cout << "Error: Book not found!" << endl;
    } else {
        if (response.find("200 OK") == string::npos) {
            cout << "Server did not respond, try again!" << endl;
            return;
        } 
        cout << "Book deleted!" << endl;
    }
}

/*  Get request to logout
*   Has as parameter the JWT token
*   Returns void, prints the response from the server
*/
void logout(string cookie) {
    int sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);

    const char *cookies[1] = {cookie.c_str()};
    char *message = compute_get_request(SERVER_IP, "/api/v1/tema/auth/logout", NULL, cookies, 1, NULL);
    send_to_server(sockfd, message);

    free(message);

    message = receive_from_server(sockfd);
    string response = string(message);
    free(message);

    close_connection(sockfd);

    // Interpret response
    if (response.find("error") != string::npos) {
        cout << "You are not authenticated" << endl;
    } else {
        if (response.find("200 OK") == string::npos) {
            cout << "Server did not respond, try again!" << endl;
            return;
        } 
        cout << "Logged out!" << endl;
    }
}

/*  Exit application and close connection to server
*/
void exit_app() {
    exit(0);
}

/*  Function that parses input from stdin
*   Allowed commands: register, login, enter_library, get_books, get_book, add_book, delete_book, logout, exit
*   Returns void , calls the function and prints the response from the server
*/
void parse_stdin()
{
    string cookie = "-1";
    string token = "-1";

    while (true) {
        string command;
        cin >> command;

        if (command == "register") {
            if(cookie != "-1") {
                cout << "Error: You are already logged in!" << endl;
                continue;
            }           
            register_user();
        } else if (command == "login") {
            if(cookie != "-1") {
                cout << "Error: You are already logged in!" << endl;
                continue;
            }
            cookie = login();
        } else if (command == "enter_library") {
            token = enter_library(cookie);
        } else if (command == "get_books") {
            get_books(token);
        } else if (command == "get_book") {
            string id;
            cout << "Book id: ";
            cin >> id;
            get_book(token, id);
        } else if (command == "add_book") {
            string title, author, genre, publisher, page_count;
            cout << "Book title: ";
            cin >> title;
            cout << "Book author: ";
            cin >> author;
            cout << "Book genre: ";
            cin >> genre;
            cout << "Book page_count: ";
            cin >> page_count;
            cout << "Book publisher: ";
            cin >> publisher;
            add_book(token, title, author, genre, page_count, publisher);
        } else if (command == "delete_book") {
            string id;
            cout << "Book id: ";
            cin >> id;
            delete_book(token, id);
        } else if (command == "logout") {
            if (cookie == "-1") {
                cout << "Error: You are not logged in!" << endl;
                continue;
            }
            
            logout(cookie);
            cookie = "-1";
            token = "-1";
        } else if (command == "exit") {
            exit_app();
        } else {
            cout << "Invalid command!" << endl;
        }
    }
}



int main () {
    // Client loop
    parse_stdin();
    
    return 0;
}