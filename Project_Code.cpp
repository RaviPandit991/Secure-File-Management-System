#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>
#include <sys/stat.h>

using namespace std;

#define KEY 'K'
#define MAX 200
class User {
public:
    string username;
    string password_hash;
    string role;
};

string hashPassword(string input) {
    string output = input;
    for (int i = 0; i < input.length(); i++)
        output[i] = input[i] + 2;
    return output;
}

int generateOTP() {
    return rand() % 9000 + 1000;
}

void log_action(string action, string file) {
    ofstream log("log.txt", ios::app);
    if (log) {
        time_t now = time(0);
        log << ctime(&now) << " - " << action << ": " << file << endl;
        log.close();
    }
}

bool checkAccess(string role, string action) {

    if (role == "admin")
        return true;

    if (role == "user") {
        if (action == "create" || action == "read" || action == "write")
            return true;
    }

    if (role == "guest") {
        if (action == "read")
            return true;
    }

    return false;
}

void encryptDecrypt(string &data) {
    for (int i = 0; i < data.length(); i++)
        data[i] ^= KEY;
}

bool isMalicious(string filename) {
    if (filename.find(".exe") != string::npos ||
        filename.find(".bat") != string::npos ||
        filename.find(".sh") != string::npos)
        return true;

    return false;
}

void add_user() {
    ofstream fp("users.txt", ios::app);

    string username, password, role;

    cout << "Enter username: ";
    cin >> username;

    cout << "Enter password: ";
    cin >> password;

    cout << "Enter role (admin/user/guest): ";
    cin >> role;

    string hash_pass = hashPassword(password);

    fp << username << " " << hash_pass << " " << role << endl;
    fp.close();

    cout << "User added successfully!\n";
}

bool login(User &loggedUser) {
    ifstream fp("users.txt");

    if (!fp) {
        cout << "No users found! Add user first.\n";
        return false;
    }

    string username, password;
    string file_user, file_pass, file_role;

    cout << "Enter username: ";
    cin >> username;

    cout << "Enter password: ";
    cin >> password;

    string hash_pass = hashPassword(password);

    while (fp >> file_user >> file_pass >> file_role) {

        if (username == file_user && hash_pass == file_pass) {

            loggedUser.username = file_user;
            loggedUser.password_hash = file_pass;
            loggedUser.role = file_role;

            int otp = generateOTP();
            cout << "OTP: " << otp << endl;

            int user_otp;
            cout << "Enter OTP: ";
            cin >> user_otp;

            if (user_otp != otp) {
                cout << "Invalid OTP!\n";
                return false;
            }

            return true;
        }
    }

    cout << "Invalid credentials!\n";
    return false;
}

void create_file(User user) {
    if (!checkAccess(user.role, "create")) {
        cout << "Access Denied!\n";
        return;
    }

    string name;
    cout << "Enter filename: ";
    cin >> name;

    if (isMalicious(name)) {
        cout << "Malicious file blocked!\n";
        return;
    }

    ofstream fp(name);
    if (fp) {
        cout << "File created!\n";
        log_action("CREATE", name);
    }
}

void write_file(User user) {
    if (!checkAccess(user.role, "write")) {
        cout << "Access Denied!\n";
        return;
    }

    string name, data;
    cout << "Enter filename: ";
    cin >> name;
    cin.ignore();

    if (isMalicious(name)) {
        cout << "Malicious file blocked!\n";
        return;
    }

    cout << "Enter content: ";
    getline(cin, data);

    encryptDecrypt(data);

    ofstream fp(name, ios::app);
    if (fp) {
        fp << data << endl;
        log_action("WRITE", name);
        cout << "Data written!\n";
    }
}

void read_file(User user) {
    if (!checkAccess(user.role, "read")) {
        cout << "Access Denied!\n";
        return;
    }

    string name, data;
    cout << "Enter filename: ";
    cin >> name;

    ifstream fp(name);
    if (fp) {
        while (getline(fp, data)) {
            encryptDecrypt(data);
            cout << data << endl;
        }
        log_action("READ", name);
    } else {
        cout << "File not found!\n";
    }
}

void delete_file(User user) {
    if (!checkAccess(user.role, "delete")) {
        cout << "Access Denied!\n";
        return;
    }

    string name;
    cout << "Enter filename: ";
    cin >> name;

    if (remove(name.c_str()) == 0) {
        cout << "Deleted!\n";
        log_action("DELETE", name);
    }
}


void rename_file(User user) {
    if (!checkAccess(user.role, "rename")) {
        cout << "Access Denied!\n";
        return;
    }

    string oldname, newname;

    cout << "Old name: ";
    cin >> oldname;

    cout << "New name: ";
    cin >> newname;

    if (rename(oldname.c_str(), newname.c_str()) == 0) {
        cout << "Renamed!\n";
        log_action("RENAME", oldname);
    }
}

void file_size() {
    string name;
    cout << "Enter filename: ";
    cin >> name;

    ifstream fp(name, ios::binary | ios::ate);
    if (fp) {
        cout << "Size: " << fp.tellg() << " bytes\n";
    }
}

void last_modified() {
    string name;
    struct stat attr;

    cout << "Enter filename: ";
    cin >> name;

    if (stat(name.c_str(), &attr) == 0) {
        cout << "Last Modified: " << ctime(&attr.st_mtime);
    }
}

void share_file(User user) {
    if (!checkAccess(user.role, "share")) {
        cout << "Access Denied!\n";
        return;
    }

    int token = rand() % 100000 + time(0) % 1000;
    cout << "Share Token: " << token << endl;
}

int main() {

    srand(time(0));

    int choice;
    User currentUser;

    while (1) {
        cout << "\n===== SYSTEM =====\n";
        cout << "1. Add User\n2. Login\n0. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        if (choice == 1) {
            add_user();
        }
        else if (choice == 2) {

            if (!login(currentUser)) {
                cout << "Login Failed!\n";
                continue;
            }

            cout << "Login Successful! Role: " << currentUser.role << endl;

            int op;

            while (1) {
                cout << "\n===== MENU =====\n";
                cout << "1.Create\n2.Write\n3.Read\n4.Delete\n5.Rename\n6.Size\n7.Modified\n8.Share\n9.Logout\n";
                cout << "Enter choice: ";
                cin >> op;

                switch (op) {
                    case 1: create_file(currentUser); break;
                    case 2: write_file(currentUser); break;
                    case 3: read_file(currentUser); break;
                    case 4: delete_file(currentUser); break;
                    case 5: rename_file(currentUser); break;
                    case 6: file_size(); break;
                    case 7: last_modified(); break;
                    case 8: share_file(currentUser); break;
                    case 9: goto logout;
                    default: cout << "Invalid choice!\n";
                }
            }

            logout:
            cout << "Logged out!\n";
        }
        else if (choice == 0) {
            exit(0);
        }
    }
}