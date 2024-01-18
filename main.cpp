#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QStringList>
#include <QInputDialog>
#include <QPainter>




struct Patient {
    QString name;
    int age;
    QString address;
    QString contactNumber;
    QString condition;
    Patient* next;
    Patient* prev;
};

Patient* head = nullptr;

void loadRecordsFromFile() {
    QFile inFile("hospital_records.txt");
    if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::information(nullptr, "Error", "No existing records found.");
        return;
    }

    QTextStream in(&inFile);
    while (!in.atEnd()) {
        Patient* newPatient = new Patient;
        in >> newPatient->name >> newPatient->age >> Qt::ws;

        newPatient->address = in.readLine().trimmed();
        newPatient->contactNumber = in.readLine().trimmed();
        newPatient->condition = in.readLine().trimmed();
        newPatient->next = head;
        if (head != nullptr) {
            head->prev = newPatient;
        }
        newPatient->prev = nullptr;
        head = newPatient;
    }

    inFile.close();
}

void saveRecordsToFile() {
    QFile outFile("hospital_records.txt");
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(nullptr, "Error", "Error opening file!");
        return;
    }

    QTextStream out(&outFile);
    Patient* current = head;
    while (current != nullptr) {
        out << current->name << " " << current->age << " " << current->address << "\n" << current->contactNumber << "\n" << current->condition << "\n";
        current = current->next;
    }

    outFile.close();
}

void addPatientRecord() {
    Patient* newPatient = new Patient;

    bool ok;
    newPatient->name = QInputDialog::getText(nullptr, "Add Patient Record", "Enter name:", QLineEdit::Normal, "", &ok);
    if (!ok || newPatient->name.isEmpty()) {
        delete newPatient;
        return;
    }

    newPatient->age = QInputDialog::getInt(nullptr, "Add Patient Record", "Enter age:", 0, 0, 100, 1, &ok);
    if (!ok) {
        delete newPatient;
        return;
    }

    newPatient->address = QInputDialog::getText(nullptr, "Add Patient Record", "Enter address:", QLineEdit::Normal, "", &ok);
    if (!ok) {
        delete newPatient;
        return;
    }

    while (true) {
        newPatient->contactNumber = QInputDialog::getText(nullptr, "Add Patient Record", "Enter contact number:", QLineEdit::Normal, "", &ok);
        if (!ok) {
            delete newPatient;
            return;
        }

        if (newPatient->contactNumber.length() == 10 && newPatient->contactNumber.toInt() > 0) {
            break;
        }

        QMessageBox::critical(nullptr, "Error", "Invalid contact number. Please enter a 10-digit number.");
    }

    newPatient->condition = QInputDialog::getText(nullptr, "Add Patient Record", "Enter condition:", QLineEdit::Normal, "", &ok);
    if (!ok) {
        delete newPatient;
        return;
    }

    newPatient->next = head;
    newPatient->prev = nullptr;
    if (head != nullptr) {
        head->prev = newPatient;
    }

    head = newPatient;

    saveRecordsToFile(); // Save after adding a record
    QMessageBox::information(nullptr, "Success", "Patient record added successfully.");
}

void displayPatientRecords() {
    if (head == nullptr) {
        QMessageBox::information(nullptr, "Information", "No records to display.");
        return;
    }

    QString records;
    Patient* current = head;
    while (current != nullptr) {
        records += "Name: " + current->name + "\nAge: " + QString::number(current->age) + "\nAddress: " + current->address + "\nContact Number: " + current->contactNumber + "\nCondition: " + current->condition + "\n\n";
        current = current->next;
    }

    QTextEdit* displayBox = new QTextEdit(nullptr);
    displayBox->setText(records);
    displayBox->setReadOnly(true);
    displayBox->show();
}

void searchPatientRecord() {
    QString searchName = QInputDialog::getText(nullptr, "Search Patient Record", "Enter name to search:");
    if (searchName.isEmpty()) return;

    Patient* current = head;
    bool found = false;
    while (current != nullptr) {
        if (current->name == searchName) {
            QString record = "Name: " + current->name + "\nAge: " + QString::number(current->age) + "\nAddress: " + current->address + "\nContact Number: " + current->contactNumber + "\nCondition: " + current->condition;
            QMessageBox::information(nullptr, "Search Results", record);
            found = true;
            break;
        }
        current = current->next;
    }

    if (!found) {
        QMessageBox::information(nullptr, "Information", "No records found for " + searchName);
    }
}

void editPatientRecord() {
    QString editName = QInputDialog::getText(nullptr, "Edit Patient Record", "Enter name to edit:");
    if (editName.isEmpty()) return;

    Patient* current = head;
    bool found = false;
    while (current != nullptr) {
        if (current->name == editName) {
            bool ok;
            current->name = QInputDialog::getText(nullptr, "Edit Patient Record", "Enter new name:", QLineEdit::Normal, current->name, &ok);
            if (!ok || current->name.isEmpty()) return;

            current->age = QInputDialog::getInt(nullptr, "Edit Patient Record", "Enter new age:", current->age, 0, 100, 1, &ok);
            if (!ok) return;

            current->address = QInputDialog::getText(nullptr, "Edit Patient Record", "Enter new address:", QLineEdit::Normal, current->address, &ok);
            if (!ok) return;

            while (true) {
                current->contactNumber = QInputDialog::getText(nullptr, "Edit Patient Record", "Enter new contact number:", QLineEdit::Normal, current->contactNumber, &ok);
                if (!ok) return;

                if (current->contactNumber.length() == 10 && current->contactNumber.toInt() > 0) {
                    break;
                }

                QMessageBox::critical(nullptr, "Error", "Invalid contact number. Please enter a 10-digit number.");
            }

            current->condition = QInputDialog::getText(nullptr, "Edit Patient Record", "Enter new condition:", QLineEdit::Normal, current->condition, &ok);
            if (!ok) return;

            found = true;
            break;
        }
        current = current->next;
    }

    if (found) {
        saveRecordsToFile(); // Save after editing
        QMessageBox::information(nullptr, "Success", "Record for " + editName + " edited successfully.");
    } else {
        QMessageBox::information(nullptr, "Information", "No records found for " + editName + ". Nothing edited.");
    }
}

void deletePatientRecord() {
    QString deleteName = QInputDialog::getText(nullptr, "Delete Patient Record", "Enter name to delete:");
    if (deleteName.isEmpty()) return;

    Patient* current = head;
    Patient* prev = nullptr;
    bool deleted = false;

    while (current != nullptr) {
        if (current->name == deleteName) {
            if (prev == nullptr) {
                head = current->next;
            } else {
                prev->next = current->next;
            }
            delete current;
            deleted = true;
            break;
        }
        prev = current;
        current = current->next;
    }

    if (deleted) {
        saveRecordsToFile(); // Save after deletion
        QMessageBox::information(nullptr, "Success", "Record for " + deleteName + " deleted successfully.");
    } else {
        QMessageBox::information(nullptr, "Information", "No records found for " + deleteName + ". Nothing deleted.");
    }
}

int main(int argc, char *argv[]) {


    QApplication app(argc, argv);
    loadRecordsFromFile(); // Load records from file at the start
    QWidget mainWindow;
    //CustomWidget mainWindow;

    mainWindow.setWindowTitle("Hospital Record System");

    mainWindow.resize(800, 600);
    // Create a button for demonstration
    //QPushButton button("u", &mainWindow);
    //button.setGeometry(100, 100, 200, 50);

    // Apply style sheet with background image
    QString styleSheet = "QWidget { background-image: url(HospitalImage.jpg); }";
    mainWindow.setStyleSheet(styleSheet);
    QLabel titleLabel("Hospital Record System");
    QFont font = titleLabel.font();
    font.setPointSize(30); // Set the font size to 20 (adjust as needed)
    titleLabel.setFont(font);


    QPushButton addRecordButton("Add Patient Record");

    QString buttonStyle = "QPushButton {"
                          "background-color: #4CAF50;"  // Green color
                          "border: none;"
                          "color: white;"
                          "padding: 15px 32px;"
                          "text-align: center;"
                          "text-decoration: none;"
                          "display: inline-block;"
                          "font-size: 16px;"
                          "margin: 4px 2px;"
                          "transition-duration: 0.4s;"
                          "cursor: pointer;"
                          "}";
    addRecordButton.setStyleSheet(buttonStyle);

    QPushButton displayRecordsButton("Display Patient Records");


    displayRecordsButton.setStyleSheet(buttonStyle);

    QPushButton searchRecordButton("Search Patient Record");

     searchRecordButton.setStyleSheet(buttonStyle);

    QPushButton editRecordButton("Edit Patient Record");

    editRecordButton.setStyleSheet(buttonStyle);

    QPushButton deleteRecordButton("Delete Patient Record");

    deleteRecordButton.setStyleSheet(buttonStyle);

    QPushButton quitButton("Quit");

    quitButton.setStyleSheet(buttonStyle);

    QVBoxLayout layout;
    layout.addWidget(&titleLabel);
    layout.addWidget(&addRecordButton);
    layout.addWidget(&displayRecordsButton);
    layout.addWidget(&searchRecordButton);
    layout.addWidget(&editRecordButton);
    layout.addWidget(&deleteRecordButton);
    layout.addWidget(&quitButton);

    mainWindow.setLayout(&layout);




    QObject::connect(&addRecordButton, &QPushButton::clicked,&addPatientRecord);
    QObject::connect(&displayRecordsButton, &QPushButton::clicked, &displayPatientRecords);
    QObject::connect(&searchRecordButton, &QPushButton::clicked, &searchPatientRecord);
    QObject::connect(&editRecordButton, &QPushButton::clicked, &editPatientRecord);
    QObject::connect(&deleteRecordButton, &QPushButton::clicked, &deletePatientRecord);

    QObject::connect(&quitButton, &QPushButton::clicked,&mainWindow,[&]()
                     {
        saveRecordsToFile(); // Save records before quitting
        mainWindow.close();
    });

    mainWindow.show();

    return app.exec();
}






/*#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "HAQT_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
    w.show();
    return a.exec();
}*/
