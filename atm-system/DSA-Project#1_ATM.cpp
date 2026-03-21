#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cctype>
#include <ctime>
#include <conio.h>
#include <windows.h>

using namespace std;

const int MAX = 1000;
const string DB_FILE = "D:\\DSA\\accountDB.dbf";
const string CHECK_DRIVES[2] = {"D:\\DSA\\checkFd.dbf", "E:\\DSA\\checkFd.dbf"};
const string DRIVES[2] = {"D:\\DSA\\bank.dbf", "E:\\DSA\\bank.dbf"};

string encryptPIN(const string &pin) {
    string encrypted = pin;
    char key = 'K';
    for (char &c : encrypted)
        c ^= key;
    return encrypted;
}

string decryptPIN(const string &encrypted) {
    return encryptPIN(encrypted);
}

// STRUCTURES
struct Account {
    string name;
    string pin;
    string phoneNum;
    string birthdate;
    int accountNo;
    int balance;
};

// ADT LIST IMPLEMENTATION
class AccountList {
private:
    Account accounts[MAX];
    int last;

public:
    AccountList() { last = -1; }

    bool isEmpty() { return last == -1; }
    bool isFull() { return last == MAX - 1; }
    int getLastIndex() { return last; }

    void insert(const Account &acc) {
        if (isFull()) {
            cout << "Account list is full.\n";
            return;
        }
        last++;
        accounts[last] = acc;
    }

    int locateByAccountNo(int accountNo) {
        for (int i = 0; i <= last; i++)
            if (accounts[i].accountNo == accountNo)
                return i;
        return -1;
    }

    int locateByPin(const string &pin) {
        for (int i = 0; i <= last; i++)
            if (accounts[i].pin == pin)
                return i;
        return -1;
    }

    Account retrieve(int index) {
        if (index < 0 || index > last) {
            cout << "Invalid index.\n";
            exit(1);
        }
        return accounts[index];
    }

    void update(int index, const Account &acc) {
        if (index >= 0 && index <= last)
            accounts[index] = acc;
    }

    void loadFromFile(const string &filename) {
        ifstream fin(filename);
        if (!fin) return;

        last = -1;
        Account acc;

        while (fin >> acc.accountNo) {
            fin.ignore();
            getline(fin, acc.name, '\t');
            fin >> acc.balance;
            fin.ignore();
            getline(fin, acc.phoneNum, '\t');
            getline(fin, acc.birthdate, '\t');
            getline(fin, acc.pin);

            acc.pin = decryptPIN(acc.pin);

            if(locateByAccountNo(acc.accountNo) == -1)
            insert(acc);
        }

        fin.close();
    }

    void saveToFile(const string &filename) {
        ofstream fout(filename);
        if(!fout){
            cout << "Error opening file for saving.\n";
            return;
        }
        for (int i = 0; i <= last; i++) {

           string encryptedPin = encryptPIN(accounts[i].pin);

            fout << accounts[i].accountNo << "\t"
                 << accounts[i].name << "\t"
                 << accounts[i].balance << "\t"
                 << accounts[i].phoneNum << "\t"
                 << accounts[i].birthdate << "\t"
                 << encryptedPin;
            if (i != last) fout << "\n";
        }
        fout.close();
    }

    Account& operator[](int index) { return accounts[index]; }
};

// GLOBAL VARIABLES
AccountList accountList;
string inputPin, reg_Pin, confirmPin, pinFromCard;
int accountNoFromCard, reg_AccountNo, reg_Bal;
int accountIndex, otherAccIndex, ctr = -1;

string readSecurePIN(const string &prompt) {
    string pin = "";
    cout << prompt;

    while (true) {
        char c = _getch();

        if (c == '\r' || c == '\n') {
            cout << "\n";

            if (pin.length() == 4) {
                return pin;
            } else {
                cout << "\n\tType 4 digits and press Enter. For 6-digit, just type 6.\n";
                return "";
            }
        } else if (c == 8) {
            if (!pin.empty()) {
                pin.pop_back();
                cout << "\b \b";
            }
            continue;
         } else if (isdigit(c) && pin.length() < 6) {
            pin.push_back(c);
            cout << "*";

            if (pin.length() == 6) {
                cout << "\n";
                return pin;
            }
        } else {
            continue;
        }
    }
}

// FUNCTION DECLARATIONS
void mainMenu();
int insertCard();
void pinEnter();
bool initialRegister();
void registerAccount();
int transaction();
bool confirmTransaction();
void saveToCard(int accountNo, const string &pin);

// MAIN PROGRAM
int main() {
    srand(time(0));
    system("color 02");
    accountList.loadFromFile(DB_FILE);

    if (insertCard()) {
        if(initialRegister()) {
         mainMenu();
        }
    }

    remove(CHECK_DRIVES[ctr].c_str());
    _getch();
    return 0;
}

// FUNCTION DEFINITIONS
int insertCard() {
    FILE *fp;
    cout << "\t\t\t\t\t=======================================\n";
    cout << "\t\t\t\t\t| < < < Please Insert Your Card > > > |\n";
    cout << "\t\t\t\t\t=======================================\n\n";
    do {
        ctr = (ctr + 1) % 2;
        fp = fopen(CHECK_DRIVES[ctr].c_str(), "w");
    } while (fp == NULL);
    fclose(fp);

    return 1;
}
bool initialRegister(){
    Account newAcc;
    system("cls");
    cout << "\t\t\t\t\t======================================\n";
    cout << "\t\t\t\t\t|         ACCOUNT REGISTRATION       |\n";
    cout << "\t\t\t\t\t======================================\n";

    cin.ignore();
    cout << "\tEnter Full Name: ";
    getline(cin, newAcc.name);
    cout << "\tEnter Phone Number: ";
    getline(cin, newAcc.phoneNum);
    cout << "\tEnter Birthdate (MM/DD/YYYY): ";
    getline(cin, newAcc.birthdate);
    cout << "\n\tEnter 4 or 6 digit PIN: ";

    reg_Pin = readSecurePIN("");
    if (reg_Pin.empty()) {
        _getch();
        return false;
    }

    cout << "\n\tConfirm new PIN: ";
    confirmPin = readSecurePIN("");

    if (confirmPin.empty()) {
        _getch();
        return false;
    } else if (reg_Pin != confirmPin) {
        cout << "\n\tPIN doesn't match\n";
        _getch();
        return false;
    }
    cout << "\n\tEnter initial deposit (minimum 5000 PHP): Php ";
    cin >> reg_Bal;

    if (reg_Bal < 5000) {
        cout << "\n\tInitial deposit must be at least 5000 PHP.\n";
        cout << "\tRegistration cancelled.\n";
        _getch();
        return false;
    }

    newAcc.pin = reg_Pin;
    newAcc.balance = reg_Bal;
    newAcc.accountNo = rand() % 90000 + 10000;

    accountList.insert(newAcc);
    accountList.saveToFile(DB_FILE);
    saveToCard(newAcc.accountNo, reg_Pin);

    cout << "\n\tAccount successfully registered!\n";
    cout << "\tYour Account Number: " << newAcc.accountNo << "\n";
    _getch();
    return true;
}

void mainMenu() {
    int choice;
    while (true) {
        system("cls");
        cout << "\t\t\t\t\t======================================\n";
        cout << "\t\t\t\t\t|        WELCOME TO TUP BANK ATM     |\n";
        cout << "\t\t\t\t\t======================================\n";
        cout << "\t\t\t\t\t[1] Register\n";
        cout << "\t\t\t\t\t[2] Login\n";
        cout << "\t\t\t\t\t[3] Exit\n";
        cout << "\t\t\t\t\tEnter choice: ";
        cin >> choice;

        switch (choice) {
            case 1:
                registerAccount();
                break;
            case 2: {
                pinEnter();
                accountIndex = accountList.locateByAccountNo(accountNoFromCard);
                if (accountIndex == -1 || accountList[accountIndex].pin != pinFromCard) {
                    cout << "\t\nAccount not found or invalid PIN.\n";
                    _getch();
                } else {
                    cout << "\n\t       Login successful!\n";
                    _getch();
                    do {
                        system("cls");
                        system("color 02");
                    } while (transaction());
                }
                break;
            }
            case 3:
                cout << "\t\nThank you for using TUP BANK ATM!\n";
                return;
            default:
                cout << "\tInvalid choice.\n";
        }
    }
}

void registerAccount(){
    if (!accountList.isEmpty()) {
        cout << "\n\t\tOnly one account can be registered on this ATM.\n";
        _getch();
        return;
    }
    Account newAcc;
    system("cls");
    cout << "\t\t\t\t\t======================================\n";
    cout << "\t\t\t\t\t|         ACCOUNT REGISTRATION       |\n";
    cout << "\t\t\t\t\t======================================\n";

    cin.ignore();
    cout << "\tEnter Full Name: ";
    getline(cin, newAcc.name);
    cout << "\tEnter Phone Number: ";
    getline(cin, newAcc.phoneNum);
    cout << "\tEnter Birthdate (MM/DD/YYYY): ";
    getline(cin, newAcc.birthdate);
    cout << "\n\tEnter 4 or 6 digit PIN: ";

    reg_Pin = readSecurePIN("");
    if (reg_Pin.empty()) {
        cout << "\tInvalid PIN. Registration cancelled.\n";
        _getch();
        return;
    }

    cout << "\n\tConfirm new PIN: ";
    confirmPin = readSecurePIN("");

    if (confirmPin.empty() || reg_Pin != confirmPin) {
        cout << "\n\tPIN doesn't match\n";
        _getch();
        return;
    }
    cout << "\n\tEnter initial deposit (minimum 5000 PHP): Php ";
    cin >> reg_Bal;

    if (reg_Bal < 5000) {
        cout << "\n\tInitial deposit must be at least 5000 PHP.\n";
        cout << "\tRegistration Failed.\n";
        _getch();
        return;
    }

    newAcc.pin = reg_Pin;
    newAcc.balance = reg_Bal;
    newAcc.accountNo = rand() % 90000 + 10000;

    accountList.insert(newAcc);
    accountList.saveToFile(DB_FILE);
    saveToCard(newAcc.accountNo, reg_Pin);

    cout << "\n\tAccount successfully registered!\n";
    cout << "\tYour Account Number: " << newAcc.accountNo << "\n";
    _getch();
}
void pinEnter() {
    ifstream fin(DRIVES[ctr]);
    if (!fin) {
        cout << "\t\nNo account data found on this card.\n";
        _getch();
        return;
    }
    int attempts = 0;
    bool found = false;
    do {
        system("COLOR 02");
        if (attempts == 3) {
            cout << "\n\t\t\t\t     LOGGING OUT . . .";
            return;
        }
        cout << "\n\t      4 or 6-digit PIN (press Enter if 4 digits): ";
        inputPin = readSecurePIN("");
        if (inputPin.empty()) {
            system("COLOR 47");
            cout << "\n\t       Invalid PIN length. Try again.              \n\n";
            _getch();
            attempts++;
            continue;
        }
        fin.clear();
        fin.seekg(0, ios::beg);
        found = false;
        int fileAccNo;
        string filePin;
        while (fin >> fileAccNo >> filePin) {
            string decrypted = decryptPIN(filePin);
            if (decrypted == inputPin) {
                accountNoFromCard = fileAccNo;
                pinFromCard = decrypted;
                found = true;
                break;
            }
        }
        if (!found) {
            system("COLOR 47");
            cout << "\n\t       Invalid Pin              \n\n";
            _getch();
        }
        attempts++;
    } while (!found);
    fin.close();
    cout << "\n\n\t       ";
    system("pause");
}

int transaction() {
    int choice;
    cout << "\t\t\t\t\t======================================\n";
    cout << "\t\t\t\t\t|         Welcome to TUP BANK        |\n";
    cout << "\t\t\t\t\t======================================\n";
    cout << "\t\t\t\t\t[1] Balance Inquiry\n";
    cout << "\t\t\t\t\t[2] Withdraw\n";
    cout << "\t\t\t\t\t[3] Deposit\n";
    cout << "\t\t\t\t\t[4] Fund Transfer\n";
    cout << "\t\t\t\t\t[5] Change PIN Code\n";
    cout << "\t\t\t\t\t[6] Exit\n";
    cout << "\t\t\t\t\tEnter choice: ";
    cin >> choice;

    switch (choice) {
        case 1:
            cout << "\nYour Balance is : Php " << accountList[accountIndex].balance << "\n";
            _getch();
            break;
        case 2:
            cout << "\nEnter amount to withdraw: Php ";
            cin >> reg_Bal;
            if (reg_Bal <= 0 || reg_Bal % 100 != 0 || reg_Bal > accountList[accountIndex].balance) {
                cout << "Invalid withdraw amount.\n";
                _getch();
            }
            else if (confirmTransaction()) {
                accountList[accountIndex].balance -= reg_Bal;
                cout << "\nSuccessfully withdrawn!\n";
                _getch();
            }
            break;
        case 3:
            cout << "\nEnter amount to deposit: Php ";
            cin >> reg_Bal;
            if (reg_Bal <= 0) {
                cout << "Invalid deposit amount.\n";
                _getch();
            }
            else if (confirmTransaction()) {
                accountList[accountIndex].balance += reg_Bal;
                cout << "\nSuccessfully deposited!\n";
                _getch();
            }
            break;
        case 4: {
            cout << "\nEnter recipient account number: ";
            cin >> reg_AccountNo;
            int idx = accountList.locateByAccountNo(reg_AccountNo);
            if (idx == -1) {
                cout << "Account not found.\n";
                _getch();
            }
            else {
                cout << "Enter transfer amount: Php ";
                cin >> reg_Bal;
                if (reg_Bal > 0 && reg_Bal <= accountList[accountIndex].balance) {
                    if (confirmTransaction()) {
                        accountList[accountIndex].balance -= reg_Bal;
                        accountList[idx].balance += reg_Bal;
                        cout << "Transfer successful!\n";
                        _getch();
                    }
                } else {
                    cout << "Invalid amount.\n";
                    _getch();
                }
            }
            break;
        }
        case 5: {
            bool validPin = false;
         do {
             cout << "\nEnter new 4 or 6-digit PIN (press Enter only after 4 digits): ";
             reg_Pin = readSecurePIN("");
             if (reg_Pin.empty()) {
                 cout << "\nInvalid PIN. Try again.\n";
                 continue;
             }
             cout << "Confirm new PIN: ";
             confirmPin = readSecurePIN("");
             if (!confirmPin.empty() && reg_Pin == confirmPin) {
                 validPin = true;
             } else {
                 cout << "\nPINs did not match or invalid length.\n";
             }
         } while (!validPin);

            accountList[accountIndex].pin = reg_Pin;
            saveToCard(accountList[accountIndex].accountNo, reg_Pin);
            cout << "\nPIN changed successfully!\n";
            _getch();
            break;
        }
        case 6:
            cout << "\nThank You For Using TUP BANK ATM SERVICE\n";
            _getch();
            return 0;
        default:
            cout << "\nInvalid choice.\n";
            _getch();
    }
    accountList.saveToFile(DB_FILE);
    return 1;
}

bool confirmTransaction() {
    char input;
    cout << "Confirm Transaction? [Y/N]: ";
    cin >> input;
    return (input == 'Y' || input == 'y');
}

 void saveToCard(int accountNo, const string &pin) {
         for (int drive = 0; drive < 2; drive++) {
             ofstream fout(DRIVES[drive]);
             if (fout.is_open()) {

                 for (int i = 0; i <= accountList.getLastIndex(); i++) {
                     int accNo = accountList[i].accountNo;
                     string currentPin = accountList[i].pin;
                     fout << accNo << "\t" << encryptPIN(currentPin) << "\n";
                 }
                 fout.close();
             }
         }
     }
