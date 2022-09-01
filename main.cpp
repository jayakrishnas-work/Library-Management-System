// Author: Jaya Krishna Soganur
// Created On: December 24, 2021
// Last Edited: September 1, 2022

// including all the related header files
#include <iostream> // for input and output operations
#include <vector> // for dynamic arrays
// headers related to Connector/C++ 8.0 JDBC 4.0 based API
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/exception.h>
using namespace std;

// function declarations
void login();
void start();
void link();
void return_books();
void borrow();
void actions();
void logout();
void signup();

// We are linking the server and application using JDBC 4.0 based API
// It is provided by Connector/C++ by MySQL DBMS
// Dependencies are as follows:
// 1. Microsoft Visual C++ Redistributable 2015 version or higher
// 2. Microsft Visual C++ (MSVC) compiler, same or higher than the MSVC used to compile JDBC library
// 3. Same build configuration, in this case 'Release' build for Windows x64 devices
// The JDBC 4.0 based API library is statically linked and MSVC++ is dynamically linked
// The target host must have MSVC++ 2015 or higher installed to smoothly run application

// defining all objects we need to connect and communicate with MySQL Server
sql::Driver* driver;
sql::Connection* connection;
sql::Statement* statement;
sql::PreparedStatement* prepared;
sql::ResultSet* result;

// defining and initializing at which address we have our database
// in this case, the deault address is "tcp://127.0.0.1:3306"
const string& address = "tcp://127.0.0.1:3306";

// 'link()' connects our appliaction to the database
void link()
{
    cout << "\t\t\t...Connecting to the database...\n\n";

    // "username', 'password': username, password of the database user with sufficient rights
    string username, password;
    cout << "Enter the 'username' of DBMS user: ";
    cin >> username; // assuming one provides us with valid username
    cout << "Enter the 'password' of DBMS user: ";
    cin >> password; // assuming one provides us with valid password
    cout << "\n";

    // lets connect to our database
    try
    {
        // think driver as tool to establish the connection
        driver = get_driver_instance();
        // 'connect()' throws an exception if something goes wrong
        connection = driver->connect(address, username, password);
    }
    catch (sql::SQLException exception)
    {
        // stdout what went wrong and exit
        cout << exception.what() << endl;

        // we need to clear the memory allocated to the below variables before exiting
        delete result;
        delete prepared;
        delete connection;

        exit(1);
    }

    // Prepare a Statement object to execute 'simple' queries;
    statement = connection->createStatement();
    
    cout << "\t\t\t...Connected to the database...\n\n\n";
}

// defining all the variables needed for the client account access
// 'name', 'username', 'password': name, username, password of the library client
string name, username, password;
//vector<string> all_users; // to store all people who took a particular book
//vector<int> all_books; // to store all books a particular client took

// 'start()' is the entry point from which the client can use the application
// though in this application, one needs to enter the username and password
// of the DBMS user credentials but, as we grow, we shall let of that dependency
void start()
{
    cout << "\t\t\t...Library Management System...\n\n\nSelect an action to perform\n\n";
    cout << "1 - Login\n2 - Register\n3 - Exit\n\nEnter here: ";
    int option = 0;
    cin >> option;

    // map the option to a set of actions
    switch (option) {
    case 1:
        login();
        break;
    case 2:
        signup();
        break;
    case 3:
        exit(0);
    default:
        cout << "\nInvalid option - Try again\n\n";
        start();
    }
}

// 'login()' authenticates and lets a client access ones account and lets one perform a set of operations
void login()
{
    // authentication of the client
    cout << "\nEnter the 'username' of the library account: ";
    cin >> username;
    cout << "Enter the 'password' of the library account: ";
    cin >> password;

    // lets access the account related to the username and password
    try
    {
        // prepared statements are derived from statements and has a few special feature
        // one is place holders '?' and lets us set his value later using member functions
        // unlike statements, which need query processing everytime, prepared statements need only one
        prepared = connection->prepareStatement("SELECT * FROM client WHERE username = ? AND password = ?");

        // set the ? values using indices, which start from 1 from left
        prepared->setString(1, username);
        prepared->setString(2, password);

        // executing the prepared query and storing the sql::ResultSet* at result 
        // result now contains pointer to the default loaction, above the 1st row
        result = prepared->executeQuery();

        // 'next()' moves 'result' cursor/pointer to next row and 
        // returns if the next row exists or not
        // if exists, one is a real client
        if (result->next())
        {
            // load the details to local memory/buffer
            name = result->getString("name");
            // got to actions
            actions();
        }
        else
        {
            cout << "\nInvalid 'username' or 'password' - Try again\n\n\n";
            start();
        }
    }
    catch (sql::SQLException exception)
    {
        // stdout what went wrong and exit
        cout << "\n" << exception.what() << endl;

        // we need to clear the memory allocated to the below variables before exiting
        delete result;
        delete prepared;
        delete connection;

        exit(1);
    }

    // clear the memory
    delete result;
    delete prepared;
    cout << "\n";
}

// 'borrow()' lets a client borrow books if present, if not show a message and returns to 'actions()'
void borrow() {
    int book_id = 0;
    cout << "\nEnter book id (fetches books better than names): ";
    cin >> book_id;
    try
    {
        prepared = connection->prepareStatement("SELECT * FROM book WHERE id = ?");
        prepared->setInt(1, book_id);
        result = prepared->executeQuery();
        if (result->next())
        {
            int count = 0;
            count = result->getInt("count");
            if (count == 0)
            {
                cout << "\nThere are no books of the id:" << book_id << " - Try Again\n";
                actions();
            }
            int x = 0;
            cout << "\nHow many books do you need? ";
            cin >> x;
            if (x > count)
            {
                cout << "\nWe only have " << count << " books of the id : " << book_id << " - Try Again\n";
                actions();
            }
            sql::PreparedStatement* prepared2 = connection->prepareStatement("SELECT * FROM borrowed WHERE client_username = ? AND book_id = ?");
            prepared2->setString(1, username);
            prepared2->setInt(2, book_id);
            /*sql::ResultSet* result2 = statement->executeQuery("SELECT * FROM borrowed WHERE client_username = " + username + " AND book_id = " + to_string(book_id));*/
            sql::ResultSet* result2 = prepared2->executeQuery();
            if (result2->next())
            {
                int countx = result->getInt("count");
                countx += x;
                statement->executeUpdate("UPDATE borrowed SET count = " + to_string(countx) + " WHERE client_username = " + username);
            }
            else
            {
                sql::PreparedStatement* prepared2 = connection->prepareStatement("INSERT INTO borrowed VALUES (?, ?, ?)");
                prepared2->setString(1, username);
                prepared2->setInt(2, book_id);
                prepared2->setInt(3, count);
                prepared2->executeQuery();
                delete prepared2;
            }
            delete result2;
            delete prepared2;
        }
        delete result;
        delete prepared;
    }
    catch (sql::SQLException exception)
    {
        // stdout what went wrong and exit
        cout << "\n" << exception.what() << endl;

        // we need to clear the memory allocated to the below variables before exiting
        delete result;
        delete prepared;
        delete connection;

        exit(1);
    }
}

// 'return_books()' lets a client return books and assumes that he is entering valid values;
void return_books()
{
    try
    {
        prepared = connection->prepareStatement("SELECT * FROM borrowed WHERE client_username = ? AND book_id = ?");
        sql::PreparedStatement* prepared2, * prepared3;
        prepared2 = connection->prepareStatement("DELETE * FROM borrowed WHERE client_username = ? AND book_id = ?");
        prepared3 = connection->prepareStatement("UPDATE borrowed SET count = ? WHERE book_id = ? AND client_username = ?");
        bool run = true;
        while (run)
        {
            int book_id, count;
            cout << "Enter book id: ";
            cin >> book_id;
            cout << "Enter number of books that of " << book_id << " (Enter 0 to exit): ";
            cin >> count;
            if (count == 0)
            {
                delete prepared;
                delete prepared2;
                delete prepared3;
                actions();
            }
            prepared->setString(1, username);
            prepared->setInt(2, book_id);
            result = prepared->executeQuery();
            if (result->next())
            {
                int countx = result->getInt("count");
                if (countx < count)
                {
                    cout << "\nThe math doesn't add up. You're returning more - Try Again\n";
                    delete prepared;
                    delete prepared2;
                    delete prepared3;
                    delete result;
                }
                else if (countx = count)
                {
                    prepared2->setString(1, username);
                    prepared2->setInt(2, book_id);
                    prepared2->executeQuery();
                }
                else
                {
                    countx -= count;
                    prepared3->setString(3, username);
                    prepared3->setInt(2, book_id);
                    prepared3->setInt(1, countx);
                    prepared3->executeQuery();
                }
            }
            else
            {
                cout << "\nThere is no record of borrow - Try Again\n";
                delete prepared;
                delete prepared2;
                delete prepared3;
                delete result;
                actions();
            }
        }
        delete prepared;
        delete prepared2;
        delete prepared3;
        delete result;
        actions();
    }
    catch (sql::SQLException exception)
    {
        // stdout what went wrong and exit
        cout << "\n" << exception.what() << endl;

        // we need to clear the memory allocated to the below variables before exiting
        delete result;
        delete prepared;
        delete connection;

        exit(1);
    }
}

// a client who has logged in needs to perform a set of actions
// 'actions' lets the client choose which action to perform
void actions()
{
    cout << "\nSelect an action to perform\n\n";
    cout << "1 - Borrow\n2 - Return\n3 - Enquiry (Not In Working)\n4 - Logout\n\nEnter here: ";
    int option = 0;
    cin >> option;

    // map the actions using switch
    switch (option) {
    case 1:
        borrow();
        break;
    case 2:
        return_books();
        break;
    case 3:
        break;
    case 4:
        logout();
        break;
    default:
        cout << "\nInvalid option\n";
        actions();
    }
}

// 'logout()' lets the client logout and 
void logout()
{
    delete result;
    delete prepared;

    cout << "\nSee you soon\n\n\n";
    start();
}

// 'singup()' lets a new client register himself into the database;
void signup()
{
    // get client details
    cout << "\nEnter your name (name) (single word): ";
    cin >> name;
    cout << "Enter your username (username) (single word): ";
    cin >> username;
    cout << "Enter your password (password) (single word): ";
    cin >> password;

    try
    {
        prepared = connection->prepareStatement("INSERT INTO client VALUES(?, ?, ?)");
        prepared->setString(1, username);
        prepared->setString(2, password);
        prepared->setString(3, name);

        if (prepared->executeUpdate())
        {
            cout << "\nRegistered\n\n";
            // as the client registered, let him login again
            start();
        }
    }
    catch (sql::SQLException exception)
    {
        // stdout what went wrong and exit
        cout << "\n" << exception.what() << endl;

        // we need to clear the memory allocated to the below variables before exiting
        delete result;
        delete prepared;
        delete connection;

        exit(1);
    }

    // clear the memory
    delete prepared;
}

int main()
{
    try
    {
        link();
        connection->setSchema("LibraryManagementSystem");
        start();
    }
    catch (sql::SQLException exception)
    {
        // stdout what went wrong and exit
        cout << exception.what() << endl;

        // we need to clear the memory allocated to the below variables before exiting
        delete result;
        delete prepared;
        delete connection;

        exit(1);
    }

    // we no longer need the connection between the application and DBMS
    // we shall clear the connections and related before we close the application
    delete result;
    delete prepared;
    delete connection;

    // driver is deleted by the Connctor/C++ 8.0 by itself
    return 0;
}