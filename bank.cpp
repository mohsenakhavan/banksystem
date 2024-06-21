#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <fstream>
#include <windows.h>
#include <cstdlib>
#include <limits>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <openssl/evp.h>
#include <openssl/err.h>
#include "figlets.h"

using namespace std;
//پروتوکل رمزگذاری sha256 برای وجود نداشتن رمز در کد 
string sha256(const std::string& input) {
    EVP_MD_CTX *mdctx;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(mdctx, input.c_str(), input.size());
    EVP_DigestFinal_ex(mdctx, hash, NULL);
    EVP_MD_CTX_free(mdctx);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

// کلاس پایه برای همه کاربران
class User {
protected:
    string username;
    string password;
    string name;
    string surname;
    string address;
    double balance;
    bool isBlocked;

public:
    // سازنده کلاس User
    User(string u, string p, string n, string s, string a, double b = 0.0)
        : username(u), password(p), name(n), surname(s), address(a), balance(b), isBlocked(false) {}
    // متد اعلام ساخته شدن اکانت
    virtual void registerUser() {
        cout << "User registered: " << getUsername() << endl;
    }
    //متد برای برگتش حالت بلاک بودن اکانت
    bool isBlock() const {
        return isBlocked;
    }
    //متد بلاک قرار دادن اکانت
    void block() {
        isBlocked = true;
    }
    //متد انبلاک قرار دادن اکانت 
    void unblock() {
        isBlocked = false;
    }
    // متد ورود به سیستم برای کاربر
    virtual bool login(const string& u, const string& p) {
        if (isBlock()) {
            cout << "Account is blocked. Please contact an employee to unblock." << endl;
            return false;
        }
        return (u == getUsername() && p == getPassword());
    }
    //متد دیدن مشخصات کلاس user
    void viewProfile() {
        cout << "Profile:\nName: " << getName() << "\nSurname: " << getSurname() << "\nAddress: " << getAddress() << "\nBalance: " << getBalance() << endl;
    }
    // متدهای دسترسی به اطلاعات کاربر
    string getUsername() const { return username; }
    string getPassword() const { return password; }
    string getName() const { return name; }
    string getSurname() const { return surname; }
    string getAddress() const { return address; }
    double getBalance() const { return balance; }
    // متد تغییر اطلاعات کاربر
    void setUsername(const string& u) { username = u; }
    void setPassword(const string& p) { password = p; }
    void setName(const string& n) { name = n; }
    void setSurname(const string& s) { surname = s; }
    void setAddress(const string& a) { address = a; }
    void setBalance(double b) { balance = b; }
};

// کلاس مشتری که از User مشتق می‌شود
class Customer : public User {
private:
    int transactionCount;
    string referrerUsername;
    int taxThreshold = 5;

public:
    // سازنده کلاس Customer
     Customer(string u, string p, string n, string s, string a, double b = 0.0, string r = "")
        : User(u, p, n, s, a, b), transactionCount(0), referrerUsername(r) {}
    // متد واریز پول به حساب
    void deposit(double amount, vector<Customer>& customers) {
    double depositAmount = amount;
    if (transactionCount >= taxThreshold) {
        depositAmount -= amount * 0.01; 
        if (getBalance() < depositAmount * 0.01) { 
            block(); 
            cout << "Insufficient balance to pay tax. Account blocked." << endl;
            return;
        }
    }
        setBalance(getBalance() + depositAmount);
        transactionCount++;
        if (!referrerUsername.empty()) {
            for (auto& customer : customers) {
                if (customer.getUsername() == referrerUsername) {
                    customer.setBalance(customer.getBalance() + amount * 0.02);
                    break;
                }
            }
        }
        cout << "Deposited: " << amount << "\nNew Balance: " << getBalance() << endl;
        logTransaction("Deposit", amount, getUsername());

    }
    // متد برداشت پول از حساب
    void withdraw(double amount) {
    double withdrawAmount = amount;
    if (getBalance() >= amount && transactionCount >= taxThreshold) {
        withdrawAmount += amount * 0.01; // افزودن 1 درصد مالیات
        if (getBalance() < withdrawAmount) { // check if customer has enough balance to pay tax
            block(); // block customer account if they don't have enough balance
            cout << "Insufficient balance to pay tax. Account blocked." << endl;
            return;
        }
    }
    if (getBalance() >= withdrawAmount) {
        setBalance(getBalance() - withdrawAmount);
        transactionCount++;
        cout << "Withdrawn: " << amount << "\nNew Balance: " << getBalance() << endl;
        logTransaction("Withdrawal", amount, getUsername());

    } else {
        cout << "Insufficient balance!" << endl;
    }
    }
    //متد انتقال پول    
    void transfer(Customer& recipient, double amount, vector<Customer>& customers) {
    double transferAmount = amount;
    if (transactionCount >= taxThreshold) {
        transferAmount += amount * 0.01; 
        if (getBalance() < transferAmount) { 
            block(); 
            cout << "Insufficient balance to pay tax. Account blocked." << endl;
            return;
        }
    }
    if (getBalance() >= transferAmount) {
        setBalance(getBalance() - transferAmount);
        recipient.deposit(amount, customers);
        transactionCount++;
        cout << "Transferred: " << amount << " to " << recipient.getUsername() << endl;
        logTransaction("Transfer", amount, getUsername(), recipient.getUsername());

    } else {
        cout << "Insufficient balance!" << endl;
    }
    }
    //متد ساخت و دخیره سازی تراکنش های سیستم
    void logTransaction(const string& operation, double amount, const string& source, const string& destination = "") {
        ofstream logFile("transaction_log.txt", ios::app);
        time_t now = time(0);
        tm* localtm = localtime(&now);
        char timestamp[64];
        strftime(timestamp, 64, "%Y-%m-%d %H:%M:%S", localtm);
        logFile << timestamp << " - " << operation << " - " << amount << " - " << source << " - " << destination << endl;
        logFile.close();
    }
    //متد نمایش موجودی فعلی
    void viewBalance() {
        cout << "Current Balance: " << getBalance() << endl;
    }
    //متد دریافت تعداد تراکنش ها 
    int getTransactionCount() const {
        return transactionCount;
    }

    vector<Customer> customers;
    //متد بلاک کردن یک مشتری 
    void blockCustomer(const string& username) {
        for (auto& customer : customers) {
            if (customer.getUsername() == username) {
                customer.block();
                cout << "Customer account blocked." << endl;
                break;
            }
        }
    }
    //متد انبلاک کردن یک مشتری 
    void unblockCustomer(const string& username) {
        for (auto& customer : customers) {
            if (customer.getUsername() == username) {
                customer.unblock();
                cout << "Customer account unblocked." << endl;
                break;
            }
        }
    }
};
// کلاس کارمند که از User مشتق می‌شود
class Employee : public User {
public:
    //سازنده کلاس کارمند
    Employee(string u, string p, string n, string s, string a)
        : User(u, p, n, s, a) {}
    //متد نمایش مشتری ها
    void viewCustomers(const vector<Customer>& customers) {
        cout << "List of Customers:" << endl;
        for (const auto& customer : customers) {
            const_cast<Customer&>(customer).viewProfile();
        }
    }
    //متد نمایش n مشتری که بیشترین مقدار بالانس را دارد 
    void viewTopNCustomers(const vector<Customer>& customers, int n) {
        auto sortedCustomers = customers;
        sort(sortedCustomers.begin(), sortedCustomers.end(), [](const Customer& a, const Customer& b) {
            return a.getBalance() > b.getBalance();
        });

        cout << "Top " << n << " Customers by Balance:" << endl;
        for (int i = 0; i < n && i < static_cast<int>(sortedCustomers.size()); i++) {
            const_cast<Customer&>(sortedCustomers[i]).viewProfile();
        }
    }
    //متد تغییر و ادیت کردن مقادیر یک مشتری 
    void editProfile(Customer& customer) {
        cout << "Editing profile of " << customer.getUsername() << endl;
        string newName, newSurname, newAddress;
        cout << "Enter new name: ";
        cin >> newName;
        cout << "Enter new surname: ";
        cin >> newSurname;
        cout << "Enter new address: ";
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        getline(cin, newAddress);
        customer.setName(newName);
        customer.setSurname(newSurname);
        customer.setAddress(newAddress);
    }
    //متد بلاک کردن یک مشتری
     void blockCustomer(Customer& customer) {
        customer.block();
        cout << "Customer account blocked." << endl;
    }
    //متد انبلاک کردن یک مشتری
    void unblockCustomer(Customer& customer) {
        customer.unblock();
        cout << "Customer account unblocked." << endl;
    }
    //متد چاپ لاگ ها از روی فایل 
    void viewTransactionLog() {
        ifstream logFile("transaction_log.txt");
        string line;
        cout << "Transaction Log:" << endl;
        while (getline(logFile, line)) {
            cout << line << endl;
        }
        logFile.close();
    }
};
//کلاس کارمند-مشتری مشتق شده از کلاس های مشتری و کارمند 
class EmployeeCustomer : public Customer, public Employee {
public:
    //سازنده کلاس کارمند-مشتری
    EmployeeCustomer(string u, string p, string n, string s, string a, double b = 0.0)
        : Customer(u, p, n, s, a, b), Employee(u, p, n, s, a) {}
    //متد واریز پول برای کارمند مشتری
    void deposit(double amount) {
        Customer::setBalance(Customer::getBalance() + amount);
        cout << "Deposited by EmployeeCustomer: " << amount << "\nNew Balance: " << Customer::getBalance() << endl;
    }
    //متد برداشت پول برای کارمند مشتری
    void withdraw(double amount) {
        if (Customer::getBalance() >= amount) {
            Customer::setBalance(Customer::getBalance() - amount);
            cout << "Withdrawn by EmployeeCustomer: " << amount << "\nNew Balance: " << Customer::getBalance() << endl;
        } else {
            cout << "Insufficient balance!" << endl;
        }
    }
    //متد انتقال پول برای کارمند مشتری
    void transfer(Customer& recipient, double amount, vector<Customer>& customers) {
    if (Customer::getBalance() >= amount) {
        Customer::setBalance(Customer::getBalance() - amount);
        recipient.deposit(amount, customers); // Pass customers vector as the second argument
        cout << "Transferred by EmployeeCustomer: " << amount << " to " << recipient.getUsername() << endl;
    } else {
        cout << "Insufficient balance!" << endl;
    }
    }
    //تعریف دوباره متد دیدن و چاپ لاگ ها برای کارمند مشتری کار اتصال را انجام میدهد 
    void viewTransactionLog() {
        Employee::viewTransactionLog();
    }
    //تعریف دوباره متد بلاک برای کارمند مشتری اتصال به متد بلاک کارمند
    void blockCustomer(Customer& customer) {
        Employee::blockCustomer(customer);
    }
    //تعریف دوباره متد انبلاک برای کارمند مشتری اتصال به متد انبلاک کارمند
    void unblockCustomer(Customer& customer) {
        Employee::unblockCustomer(customer);
    }
    //تعریف دوباره مشاهده n مشتری بالا تر و اتصال به متد در کارمند
    void viewTopNCustomers(const vector<Customer>& customers, int n) {
    Employee::viewTopNCustomers(customers, n);
    }
};

class Admin : public User {
public:
    // سازنده کلاس Admin
    Admin(string u, string p, string n, string s, string a)
        : User(u, p, n, s, a) {}
    // متد دادن هدیه تصادفی به مشتریان
    void giveRandomGifts(vector<Customer>& customers) {
    cout << "Admin giving random gifts to customers" << endl;
    srand(static_cast<unsigned int>(time(0))); // تنظیم seed برای تولید اعداد تصادفی

    int numCustomers = customers.size();
    int numGifts = rand() % numCustomers + 1; // تعداد هدایا بین ۱ تا تعداد مشتریان تصادفی

    for (int i = 0; i < numGifts; i++) {
        int randomIndex = rand() % numCustomers; // Random customer index
        double gift = static_cast<double>(rand()) / RAND_MAX * 1000.0; // Random gift amount between 0 and 1000
        customers[randomIndex].setBalance(customers[randomIndex].getBalance() + gift);
        cout << "Gifted " << gift << " to " << customers[randomIndex].getUsername() << endl;
    }
    }
    // متد حذف کاربر یا همان مشتری
    void deleteUser(vector<Customer>& customers, const string& username) {
        auto it = remove_if(customers.begin(), customers.end(), [&](const Customer& customer) {
            return customer.getUsername() == username;
        });

        if (it != customers.end()) {
            customers.erase(it, customers.end());
            cout << "User " << username << " deleted" << endl;
        } else {
            cout << "User " << username << " not found" << endl;
        }
    }
};
// تابع اصلی برنامه
int main() {
   vector<Customer> customers;
   
   string SHA256_PASS = "4cf6829aa93728e8f3c97df913fb1bfa95fe5810e2933a05943f8312a98d9cf2";
   //بخش تعریف و تنظیم مشخصات و خود کارمند ادمین و کارمن-مشتری
   Admin admin("sa", SHA256_PASS, "Admin", "Owner", "Admin Address");
   Employee emp1("se", "se", "Emp", "One", "Employee Address");
   EmployeeCustomer empCust1("sc", "sc", "EmpCust", "One", "EmployeeCustomer Address");
   //حلقه تکرار کلی برای نمایش منو و محیط کاربری
   int choice;
   do {
       Sleep(4000);
       system("cls");
       cout << art << endl;
       cout << "Bank Management System\n";
       cout << "1. Register Customer\n";
       cout << "2. Login Customer\n";
       cout << "3. Login Employee\n";
       cout << "4. Login Employee-Customer\n";                          //بخش چاپ منو اصلی 
       cout << "5. Login Admin\n";
       cout << "6. Exit\n";
       cout << "Enter your choice: ";
       cin >> choice;

       string username, password, name, surname, address,referrerUsername;
       double amount;

       switch (choice) {
           case 1:
                system("cls");
                cout << art1 << endl;
                cout << "Enter Username: ";
                cin >> username;
                cout << "Enter Password: ";
                cin >> password;
                cout << "Enter Name: ";
                cin >> name;                                      // بخش ریجستر کردن مشتری
                cout << "Enter Surname: ";     
                cin >> surname;
                cout << "Enter Address: ";
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                getline(cin, address);

                char hasReferrer;
                cout << "Do you have a referrer? (y/n): ";
                cin >> hasReferrer;

                if (hasReferrer == 'y' || hasReferrer == 'Y') {
                    cout << "Enter Referrer Username: ";
                    cin >> referrerUsername;
                    customers.emplace_back(username, password, name, surname, address, 0.0, referrerUsername);
                } else {
                    customers.emplace_back(username, password, name, surname, address, 0.0, "");
                }

                customers.back().registerUser();
                break;

           case 2:
               system("cls");
               cout << art2 << endl;
               cout << "Enter Username: ";
               cin >> username;                                                                    
               cout << "Enter Password: ";
               cin >> password;

               for (auto& customer : customers) {
                   if (customer.login(username, password)) {
                       cout << "Login Successful!" << endl;
                       int customerChoice;
                       do {
                           Sleep(4000);
                           system("cls");
                           cout << art2p << endl;
                           cout << "Customer Menu\n";
                           cout << "1. Deposit\n";
                           cout << "2. Withdraw\n";
                           cout << "3. Transfer\n";
                           cout << "4. View Balance\n";                                              // بخش ورود مشتری
                           cout << "5. Logout\n";
                           cout << "Enter your choice: ";
                           cin >> customerChoice;

                           switch (customerChoice) {
                               case 1:
                                    system("cls");
                                    cout << "Enter amount to deposit: ";
                                    cin >> amount;                                                    //بخش واریز مشتری
                                    customer.deposit(amount, customers);
                                    break;
                               case 2:
                                   system("cls");
                                   cout << "Enter amount to withdraw: ";
                                   cin >> amount;                                                     //بخش برداشت مشتری
                                   customer.withdraw(amount); 
                                   break;
                               case 3:
                                    system("cls");
                                    cout << "Enter recipient's username: ";
                                    cin >> username;
                                    cout << "Enter amount to transfer: ";                             // بخش انتقال مشتری
                                    cin >> amount;
                                    for (auto& recipient : customers) {
                                        if (recipient.getUsername() == username) {
                                            customer.transfer(recipient, amount, customers);
                                            break;
                                        }
                                    }
                                    break;
                                   break;
                               case 4:
                                   system("cls");
                                   customer.viewBalance();                                             // مشاهد هموجودی مشتری
                                   break;
                               case 5:
                                   system("cls");
                                   cout << "Logging out...\n";
                                   break;
                               default:
                                   system("cls");
                                   cout << "Invalid choice!\n";
                                   break;
                           }
                       } while (customerChoice != 5);
                       break;
                   }
               }
               break;

           case 3:
               system("cls");
               cout << art3 << endl;
               cout << "Enter Username: ";
               cin >> username;
               cout << "Enter Password: ";
               cin >> password;

               if (emp1.login(username, password)) {
                   cout << "Login Successful!" << endl;
                   int employeeChoice;
                   do {
                       Sleep(4000);
                       system("cls");
                       cout << artp3 << endl;
                       cout << "Employee Menu\n";
                       cout << "1. View Customers\n";
                       cout << "2. View Top Customers\n";
                       cout << "3. Edit Customer Profile\n";
                       cout << "4. Block Customer Account\n";                                           // بخش ورود و منو کارمند 
                       cout << "5. unblock Customer Account\n";
                       cout << "6. View transaction log\n";
                       cout << "7. Logout\n";
                       cout << "Enter your choice: ";
                       cin >> employeeChoice;

                       switch (employeeChoice) {
                           case 1:
                               system("cls");
                               emp1.viewCustomers(customers);                                           // بخش دیدن مشتری ها
                               break;
                           case 2:
                                system("cls");
                                int topNCustomers;
                                cout << "Enter the number of top customers to view: ";                  // بخش دیدن مشتری های برتر 
                                cin >> topNCustomers;
                                emp1.viewTopNCustomers(customers, topNCustomers);
                                break;
                           case 3:
                               system("cls");
                               cout << "Enter Customer Username: ";
                               cin >> username;
                               for (auto& customer : customers) {
                                   if (customer.getUsername() == username) {                            // بخش ادیت کردن مشتری
                                       emp1.editProfile(customer);
                                       break;
                                   }
                               }
                               break;
                           case 4:
                               system("cls");
                               cout << "Enter Customer Username to block: ";
                                cin >> username;
                                for (auto& customer : customers) {                                      // بخش بلاک کردن مشتری 
                                    if (customer.getUsername() == username) {
                                        emp1.blockCustomer(customer);
                                        break;
                                    }
                                }
                                break;
                           case 5:
                               system("cls");
                               cout << "Enter Customer Username to unblock: ";
                                cin >> username;
                                for (auto& customer : customers) {                                      // بخش انبلاک کردن مشتری
                                    if (customer.getUsername() == username) {
                                        emp1.unblockCustomer(customer);
                                        break;
                                    }
                                }
                                break;
                           case 6:
                               system("cls");
                               emp1.viewTransactionLog();                                              // یخش مشاهده لاگ فایل
                               break;     
                           case 7:
                               system("cls");
                               cout << "Logging out...\n";
                               break;
                           default:
                               system("cls");
                               cout << "Invalid choice!\n";
                               break;
                       }
                   } while (employeeChoice != 7);
               } else {
                   cout << "Login Failed!" << endl;
               }
               break;

           case 4:
               system("cls");
               cout << art4 << endl;
               cout << "Enter Username: ";
               cin >> username;
               cout << "Enter Password: ";
               cin >> password;

                       if (empCust1.Customer::login(username, password)) {
                   cout << "Login Successful!" << endl;
                   int empCustChoice;
                   do {
                       Sleep(4000);
                       system("cls");
                       cout << artp4 << endl;
                       cout << "Employee-Customer Menu\n";
                       cout << "1. Deposit\n";
                       cout << "2. Withdraw\n";
                       cout << "3. Transfer\n";
                       cout << "4. View Balance\n";
                       cout << "5. View Customers\n";
                       cout << "6. View Top Customers\n";                                               // یخش ورود و منوی کارمند مشتری
                       cout << "7. Edit Customer Profile\n";
                       cout << "8. Block Customer Account\n";
                       cout << "9. unblock Customer Account\n";
                       cout << "10. View transaction log\n";
                       cout << "11. Logout\n";
                       cout << "Enter your choice: ";
                       cin >> empCustChoice;

                       switch (empCustChoice) {
                           case 1:
                               system("cls");
                               cout << "Enter amount to deposit: ";
                               cin >> amount;                                                         // واریز کارمد مشتری
                               empCust1.deposit(amount);
                               break;
                           case 2:
                               system("cls");
                               cout << "Enter amount to withdraw: ";
                               cin >> amount;                                                        // برداشت کارمند مشتری
                               empCust1.withdraw(amount);
                               break;
                           case 3:
                                system("cls");
                                cout << "Enter recipient's username: ";
                                cin >> username;
                                cout << "Enter amount to transfer: ";                                // انتقال کارمند مشتری
                                cin >> amount;
                                for (auto& recipient : customers) {
                                    if (recipient.getUsername() == username) {
                                        empCust1.transfer(recipient, amount, customers);
                                        break;
                                    }
                                }
                                break;
                           case 4:
                               system("cls");
                               empCust1.viewBalance();                                              // دیدن موجودی کارمند مشتری
                               break;
                           case 5:
                               system("cls");
                               empCust1.viewCustomers(customers);                                   // دیدن لیست مشتری های دیگر 
                               break;
                           case 6:
                                system("cls");
                                int topNCustomers;
                                cout << "Enter the number of top customers to view: ";
                                cin >> topNCustomers;                                               // دیدن مشتری های برتر 
                                empCust1.viewTopNCustomers(customers, topNCustomers);
                                break;
                           case 7: 
                               system("cls");
                               cout << "Enter Customer Username: ";
                               cin >> username;
                               for (auto& customer : customers) {                                   // ادیت کردن مشخصات مشتری ها
                                   if (customer.getUsername() == username) {
                                       empCust1.editProfile(customer);
                                       break;
                                   }
                               }
                               break;
                           case 8:
                                system("cls");
                                cout << "Enter Customer Username to block: ";
                                cin >> username;
                                for (auto& customer : customers) {                                 // بلاک کردن مشتری
                                    if (customer.getUsername() == username) {
                                        empCust1.blockCustomer(customer);
                                        break;
                                    }
                                }
                                break;
                           case 9:
                           system("cls");
                           cout << "Enter Customer Username to unblock: ";
                            cin >> username;
                            for (auto& customer : customers) {                                     // انبلاک کردن مشتری
                                if (customer.getUsername() == username) {
                                    empCust1.unblockCustomer(customer);
                                    break;
                                }
                            }
                            break;
                           case 10:
                                system("cls");
                                empCust1.viewTransactionLog();                                    // دیدن لاگ فایل و چاپ 
                                break;    
                           case 11:
                               system("cls");
                               cout << "Logging out...\n";
                               break;
                           default:
                               system("cls");
                               cout << "Invalid choice!\n";
                               break;
                       }
                   } while (empCustChoice != 11);
               } else {
                   cout << "Login Failed!" << endl;
               }
               break;

           case 5:
                system("cls");
                cout << art5 << endl;
                cout << "Enter Username: ";
                cin >> username;
                cout << "Enter Password: ";
                cin >> password;

                if (admin.login(username, sha256(password))) {
                    cout << "Login Successful!" << endl;
                    int adminChoice;
                    do {
                        Sleep(4000);
                        system("cls");                                                         // بخش ادمین و ورود و منو ادمین 
                        cout << artp5 << endl;
                        cout << "Admin Menu\n";
                        cout << "1. Give Random Gifts\n";
                        cout << "2. Delete User\n";
                        cout << "3. Logout\n";
                        cout << "Enter your choice: ";
                        cin >> adminChoice;

                        switch (adminChoice) {
                            case 1:
                                system("cls");
                                admin.giveRandomGifts(customers);                              //دادن مبلغ رندوم به کاربر رندوم  
                                break;
                            case 2:
                                system("cls");
                                cout << "Enter Username to delete: ";
                                cin >> username;                                              // دیلیت کردن کاربر ها
                                admin.deleteUser(customers, username);
                                break;
                            case 3:
                                system("cls");
                                cout << "Logging out...\n";
                                break;
                            default:
                                system("cls");
                                cout << "Invalid choice!\n";
                                break;
                        }
                    } while (adminChoice != 3);
                } else {
                    cout << "Login Failed!" << endl;
                }
                break;

           case 6:
               system("cls");
               cout << "Exiting..." << endl;
               break;

           default:
               system("cls");
               cin.ignore();
               cout << "Invalid Choice!" << endl;
               break;
       }
   } while (choice != 6);
    Sleep(5000);
   return 0;
}

// C++ compile and flag commands:
// g++ -o bank.exe bank.cpp -L/usr/lib -lssl -lcrypto