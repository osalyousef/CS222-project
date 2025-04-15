#include "AccInfo.h"
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <chrono>
#include "Admin/Admin.h"
using namespace std;
// Accounts class implementation
Accounts::Accounts() {
    balance = 0;
    Historyhead = NULL;
}

Accounts::Accounts(string password, string firstName, string lastName, int phoneNum) {
    this->password = password;
    this->fullName = firstName + " " + lastName;  // Concatenate first and last name
    this->phoneNum = phoneNum;
    this->balance = 0;
    Historyhead = NULL;

    // Generate random accountId with 8 digits
    this->accountId = generateRandomAccountId();
}

int Accounts::generateRandomAccountId() {
    return rand() % 90000000 + 10000000;  // Generate an 8-digit accountId
}

int Accounts::getAccountId() { return accountId; }
string Accounts::getPassword() { return password; }
float Accounts::getBalance() { return balance; }
string Accounts::getName() { return fullName; }
int Accounts::getPhoneNum() { return phoneNum; }

void Accounts::deposit(int amount) {
    balance += amount;
    cout << "Deposited: " << amount << endl;
    addTrans(amount, "deposit");
}

void Accounts::withdraw(int amount) {
    if (balance < amount) {
        cout << "Insufficient balance.\n";
        return;
    }
    balance -= amount;
    cout << "Withdrawn: " << amount << endl;
    addTrans(amount, "withdraw");
}

void Accounts::transfer(int amount, Accounts& receiver, string direction) {
    if (balance < amount) {
        cout << "Insufficient balance.\n";
        return;
    }
    balance -= amount;
    receiver.balance += amount;

    // Add transaction for the sender (with last 4 digits of the receiver's account ID)
    addTrans(amount, "transfer", receiver.getName(), receiver.getAccountId(), "to");

    // Add transaction for the receiver (with last 4 digits of the sender's account ID)
    receiver.addTrans(amount, "transfer", getName(), getAccountId(), "from");

    cout << "Transferred " << amount << " to " << receiver.getName() << endl;
}

void Accounts::addTrans(int amount, string type, string name, int idLast4, string direction) {
    // Get current date and time
    auto now = chrono::system_clock::to_time_t(chrono::system_clock::now());
    string dateTime = ctime(&now);  // Convert time to string (includes a newline at the end)

    // Remove the trailing newline character added by ctime
    dateTime.pop_back();  // This removes the newline character at the end of the dateTime string

    // Get the last 4 digits of the account ID
    int last4 = idLast4 % 10000;  // This guarantees it will only show the last 4 digits

    // Create a new transaction
    TransHistory* temp = new TransHistory;
    temp->amount = amount;
    temp->type = type;
    temp->senderName = name;
    temp->last4Num_Id = last4;  // Store only the last 4 digits
    temp->direction = direction;
    temp->dateTime = dateTime;
    temp->next = NULL; // Set the next pointer to NULL initially

    if (Historyhead == NULL) {
        Historyhead = temp;  // If the list is empty, set the first element
    } else {
        TransHistory* current = Historyhead;
        while (current->next != NULL) {
            current = current->next;  // Traverse the list to find the last element
        }
        current->next = temp;  // Add the new transaction to the end
    }
}

void Accounts::printTransHistory() {
    TransHistory* current = Historyhead;
    if (current == NULL) {
        cout << "No transaction history.\n";
        return;
    }

    // Traverse the linked list and print each transaction
    while (current != NULL) {
        cout << "[" << current->dateTime << "] ";  // Print date and time of the transaction

        if (current->type == "transfer") {
            cout << "Transfer " << current->direction << " (" << current->last4Num_Id << ") "
                 << current->senderName << ": " << current->amount << endl;
        } else {
            cout << "You " << current->type << ": " << current->amount << endl;
        }

        current = current->next;  // Move to the next transaction in the linked list
    }
}

// AccountMap class implementation
int AccountMap::hashFunction(int phoneNum) {
    return phoneNum % TABLE_SIZE;  // Use phone number as the key for hashing
}

AccountMap::AccountMap() {
    for (int i = 0; i < TABLE_SIZE; i++) table[i] = nullptr;
}

void AccountMap::insertAccount(Accounts acc) {
    int key = acc.getPhoneNum();  // Use phone number as the key
    int index = hashFunction(key);

    AccountEntry* newEntry = new AccountEntry{ key, acc, nullptr };

    if (table[index] == nullptr) {
        table[index] = newEntry;
    } else {
        AccountEntry* current = table[index];
        while (current->next != nullptr)
            current = current->next;
        current->next = newEntry;
    }
}

Accounts* AccountMap::getAccount(int phoneNum) {
    int index = hashFunction(phoneNum);  // Use phone number to calculate index
    AccountEntry* current = table[index];
    while (current != nullptr) {
        if (current->account.getPhoneNum() == phoneNum)  // Check if the phone number matches
            return &(current->account);  // Return the account object if match found
        current = current->next;
    }
    return nullptr;  // Account not found
}

// Global AccountMap definition
AccountMap bankSystem;

// UI Function implementation
void ui() {
    Admin admin(&bankSystem); // Create the admin object
    
    int choice;
    while (true) {
        cout << "\n===== Bank Menu =====\n";
        cout << "1. Create Account\n";
        cout << "2. Log In\n";
        cout << "3. Admin Panel\n";  // Added admin option
        cout << "4. Exit\n";
        cout << "Choose: ";
        cin >> choice;

        if (choice == 1) {
            string firstName, lastName, pass;
            int phone;
            cout << "Enter First Name: ";
            cin.ignore();
            getline(cin, firstName);
            cout << "Enter Last Name: ";
            getline(cin, lastName);
            cout << "Enter Password: ";
            getline(cin, pass);
            cout << "Enter Phone Number: ";
            cin >> phone;

            if (bankSystem.getAccount(phone)) {
                cout << "Account already exists.\n";
                continue;
            }

            Accounts newAcc(pass, firstName, lastName, phone);
            bankSystem.insertAccount(newAcc);
            cout << "Account created successfully. Your Account ID is: " << newAcc.getAccountId() << endl;

        } else if (choice == 2) {
            int phone;
            string pass;
            cout << "Enter Phone Number: ";
            cin >> phone;
            cout << "Enter Password: ";
            cin.ignore();
            getline(cin, pass);

            Accounts* acc = bankSystem.getAccount(phone);
            if (!acc || acc->getPassword() != pass) {
                cout << "Invalid credentials.\n";
                continue;
            }

            int subChoice;
            while (true) {
                cout << "\n--- Welcome, " << acc->getName() << " ---\n";
                cout << "1. Deposit\n2. Withdraw\n3. Transfer\n4. View History\n5. Logout\nChoose: ";
                cin >> subChoice;

                if (subChoice == 1) {
                    int amount;
                    cout << "Amount to deposit: ";
                    cin >> amount;
                    acc->deposit(amount);
                } else if (subChoice == 2) {
                    int amount;
                    cout << "Amount to withdraw: ";
                    cin >> amount;
                    acc->withdraw(amount);
                } else if (subChoice == 3) {
                    int toPhone, amount;
                    cout << "Receiver's Phone Number: ";
                    cin >> toPhone;
                    cout << "Amount: ";
                    cin >> amount;

                    Accounts* receiver = bankSystem.getAccount(toPhone);
                    if (!receiver) {
                        cout << "Receiver not found.\n";
                        continue;
                    }
                    // Transfer from the current account
                    acc->transfer(amount, *receiver, "to");
                } else if (subChoice == 4) {
                    acc->printTransHistory();
                } else if (subChoice == 5) {
                    break;
                } else {
                    cout << "Invalid option.\n";
                }
            }
        } else if (choice == 3) {
            // Admin access
            admin.adminPanel();
        } else if (choice == 4) {
            cout << "Goodbye!\n";
            break;
        } else {
            cout << "Invalid choice.\n";
        }
    }
}
