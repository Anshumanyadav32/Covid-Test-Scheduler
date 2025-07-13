#include "covid_test_scheduler.h"
#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QInputDialog>
#include <QHeaderView>
#include <QSplitter>
#include <QStatusBar>
#include <QMenuBar>
#include <QAction>
#include <QFont>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <algorithm>
#include <QDateEdit>

// TimeSlot Implementation
TimeSlot::TimeSlot(int id, const QString &time, const QString &date)
    : id_(id), time_(time), date_(date), is_booked_(false) {}

bool TimeSlot::operator>(const TimeSlot &other) const
{
    QDateTime this_datetime = QDateTime::fromString(getDateTime(), "yyyy-MM-dd hh:mm");
    QDateTime other_datetime = QDateTime::fromString(other.getDateTime(), "yyyy-MM-dd hh:mm");
    return this_datetime > other_datetime;
}

bool TimeSlot::operator<(const TimeSlot &other) const
{
    QDateTime this_datetime = QDateTime::fromString(getDateTime(), "yyyy-MM-dd hh:mm");
    QDateTime other_datetime = QDateTime::fromString(other.getDateTime(), "yyyy-MM-dd hh:mm");
    return this_datetime < other_datetime;
}

bool TimeSlot::operator==(const TimeSlot &other) const
{
    return id_ == other.id_;
}

// Patient Implementation
Patient::Patient(const QString &name, int age, std::shared_ptr<TimeSlot> assignedSlot)
    : name_(name), age_(age), assigned_slot_(assignedSlot)
{
    booking_time_ = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

// CovidTestScheduler Implementation
CovidTestScheduler::CovidTestScheduler(QWidget *parent)
    : QMainWindow(parent), next_slot_id_(1)
{
    setupUI();
    setupMenuBar();
    setupStatusBar();

    // Initialize with some sample slots
    addSampleSlots();

    // Setup timer for datetime updates
    datetime_timer_ = new QTimer(this);
    connect(datetime_timer_, &QTimer::timeout, this, &CovidTestScheduler::updateDateTime);
    datetime_timer_->start(1000); // Update every second

    updateDateTime();
    refreshDisplay();
}

CovidTestScheduler::~CovidTestScheduler()
{
    // Qt handles cleanup automatically
}

void CovidTestScheduler::setupUI()
{
    setWindowTitle("Covid Test Center Scheduler");
    setMinimumSize(1000, 700);

    central_widget_ = new QWidget(this);
    setCentralWidget(central_widget_);

    main_layout_ = new QVBoxLayout(central_widget_);

    // Add date selector for booking
    QHBoxLayout *date_select_layout = new QHBoxLayout();
    date_select_layout->addWidget(new QLabel("Select Date:"));
    date_select_edit_ = new QDateEdit(QDate::currentDate());
    date_select_edit_->setDisplayFormat("yyyy-MM-dd");
    date_select_edit_->setCalendarPopup(true);
    date_select_layout->addWidget(date_select_edit_);
    main_layout_->addLayout(date_select_layout);
    connect(date_select_edit_, &QDateEdit::dateChanged, this, &CovidTestScheduler::refreshDisplay);

    // Create splitter for better layout management
    QSplitter *main_splitter = new QSplitter(Qt::Horizontal, this);
    main_layout_->addWidget(main_splitter);

    // Left panel - Input controls
    QWidget *left_panel = new QWidget();
    left_panel->setMaximumWidth(350);
    QVBoxLayout *left_layout = new QVBoxLayout(left_panel);

    // Add Slot Group
    add_slot_group_ = new QGroupBox("Add New Time Slot");
    QGridLayout *add_slot_layout = new QGridLayout(add_slot_group_);

    add_slot_layout->addWidget(new QLabel("Date (YYYY-MM-DD):"), 0, 0);
    date_input_ = new QLineEdit();
    date_input_->setPlaceholderText("2024-01-15");
    date_input_->setText(QDate::currentDate().toString("yyyy-MM-dd"));
    add_slot_layout->addWidget(date_input_, 0, 1);

    add_slot_layout->addWidget(new QLabel("Time (HH:MM):"), 1, 0);
    time_input_ = new QLineEdit();
    time_input_->setPlaceholderText("09:00");
    add_slot_layout->addWidget(time_input_, 1, 1);

    add_slot_button_ = new QPushButton("Add Slot");
    add_slot_button_->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");
    add_slot_layout->addWidget(add_slot_button_, 2, 0, 1, 2);

    left_layout->addWidget(add_slot_group_);

    // Book Patient Group
    book_patient_group_ = new QGroupBox("Book Patient Appointment");
    QGridLayout *book_layout = new QGridLayout(book_patient_group_);

    book_layout->addWidget(new QLabel("Patient Name:"), 0, 0);
    patient_name_input_ = new QLineEdit();
    patient_name_input_->setPlaceholderText("Enter patient name");
    book_layout->addWidget(patient_name_input_, 0, 1);

    book_layout->addWidget(new QLabel("Age:"), 1, 0);
    patient_age_input_ = new QSpinBox();
    patient_age_input_->setRange(1, 120);
    patient_age_input_->setValue(25);
    book_layout->addWidget(patient_age_input_, 1, 1);

    book_layout->addWidget(new QLabel("Available Slots:"), 2, 0);
    available_slots_combo_ = new QComboBox();
    book_layout->addWidget(available_slots_combo_, 2, 1);

    book_slot_button_ = new QPushButton("Book Appointment");
    book_slot_button_->setStyleSheet("QPushButton { background-color: #2196F3; color: white; font-weight: bold; }");
    book_layout->addWidget(book_slot_button_, 3, 0, 1, 2);

    left_layout->addWidget(book_patient_group_);

    // Action buttons
    QHBoxLayout *action_layout = new QHBoxLayout();

    view_bookings_button_ = new QPushButton("View All Bookings");
    view_bookings_button_->setStyleSheet("QPushButton { background-color: #FF9800; color: white; }");
    action_layout->addWidget(view_bookings_button_);

    cancel_slot_button_ = new QPushButton("Cancel Booking");
    cancel_slot_button_->setStyleSheet("QPushButton { background-color: #f44336; color: white; }");
    action_layout->addWidget(cancel_slot_button_);

    refresh_button_ = new QPushButton("Refresh");
    refresh_button_->setStyleSheet("QPushButton { background-color: #9C27B0; color: white; }");
    action_layout->addWidget(refresh_button_);

    left_layout->addLayout(action_layout);
    left_layout->addStretch();

    // Add a label to show the number of available slots live
    available_slots_count_label_ = new QLabel();
    QFont countFont;
    countFont.setBold(true);
    available_slots_count_label_->setFont(countFont);
    main_layout_->addWidget(available_slots_count_label_);

    main_splitter->addWidget(left_panel);

    // Right panel - Display areas
    QWidget *right_panel = new QWidget();
    QVBoxLayout *right_layout = new QVBoxLayout(right_panel);

    // Available Slots Group
    slots_group_ = new QGroupBox("Available Time Slots (Min-Heap Order)");
    QVBoxLayout *slots_layout = new QVBoxLayout(slots_group_);

    available_slots_list_ = new QListWidget();
    available_slots_list_->setStyleSheet("QListWidget { background-color: #222; color: #fff; }");
    slots_layout->addWidget(available_slots_list_);

    right_layout->addWidget(slots_group_);

    // Bookings Group
    bookings_group_ = new QGroupBox("Patient Bookings");
    QVBoxLayout *bookings_layout = new QVBoxLayout(bookings_group_);

    bookings_table_ = new QTableWidget();
    bookings_table_->setColumnCount(5);
    QStringList headers;
    headers << "Slot ID" << "Patient Name" << "Age" << "Date & Time" << "Booking Time";
    bookings_table_->setHorizontalHeaderLabels(headers);
    bookings_table_->horizontalHeader()->setStretchLastSection(true);
    bookings_table_->setAlternatingRowColors(true);
    bookings_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    bookings_layout->addWidget(bookings_table_);

    right_layout->addWidget(bookings_group_);

    main_splitter->addWidget(right_panel);
    main_splitter->setStretchFactor(0, 0);
    main_splitter->setStretchFactor(1, 1);

    // Connect signals
    connect(add_slot_button_, &QPushButton::clicked, this, &CovidTestScheduler::addSlot);
    connect(book_slot_button_, &QPushButton::clicked, this, &CovidTestScheduler::bookSlot);
    connect(view_bookings_button_, &QPushButton::clicked, this, &CovidTestScheduler::viewBookings);
    connect(cancel_slot_button_, &QPushButton::clicked, this, &CovidTestScheduler::cancelSlot);
    connect(refresh_button_, &QPushButton::clicked, this, &CovidTestScheduler::refreshDisplay);
}

void CovidTestScheduler::setupMenuBar()
{
    QMenuBar *menuBar = this->menuBar();

    // File menu
    QMenu *fileMenu = menuBar->addMenu("&File");

    QAction *exitAction = new QAction("E&xit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(exitAction);

    // Help menu
    QMenu *helpMenu = menuBar->addMenu("&Help");

    QAction *aboutAction = new QAction("&About", this);
    connect(aboutAction, &QAction::triggered, [this]()
            { QMessageBox::about(this, "About Covid Test Scheduler",
                                 "Covid Test Center Scheduler v1.0\n\n"
                                 "A C++ Qt application using min-heap data structure\n"
                                 "for efficient Covid test slot scheduling.\n\n"
                                 "Features:\n"
                                 "• Min-heap based slot allocation\n"
                                 "• Patient booking management\n"
                                 "• Real-time slot availability\n"
                                 "• Booking cancellation\n\n"
                                 "Built with Qt6 and C++ STL"); });
    helpMenu->addAction(aboutAction);
}

void CovidTestScheduler::setupStatusBar()
{
    status_label_ = new QLabel("Ready");
    datetime_label_ = new QLabel();

    statusBar()->addWidget(status_label_);
    statusBar()->addPermanentWidget(datetime_label_);
}

void CovidTestScheduler::addSampleSlots()
{
    // Add some sample slots for demonstration
    QDate today = QDate::currentDate();
    QStringList times = {"09:00", "09:30", "10:00", "10:30", "11:00", "11:30", "14:00", "14:30", "15:00", "15:30"};

    for (int day = 0; day < 3; ++day)
    {
        QString date = today.addDays(day).toString("yyyy-MM-dd");
        for (const QString &time : times)
        {
            auto slot = std::make_shared<TimeSlot>(next_slot_id_++, time, date);
            all_slots_.push_back(slot);
            slotsByDate_[date].push(slot);
        }
    }
}

void CovidTestScheduler::addSlot()
{
    QString date = date_input_->text().trimmed();
    QString time = time_input_->text().trimmed();

    if (date.isEmpty() || time.isEmpty())
    {
        QMessageBox::warning(this, "Input Error", "Please enter both date and time.");
        return;
    }

    // Validate date format
    QDate qdate = QDate::fromString(date, "yyyy-MM-dd");
    if (!qdate.isValid())
    {
        QMessageBox::warning(this, "Date Error", "Please enter date in YYYY-MM-DD format.");
        return;
    }

    // Validate time format
    QTime qtime = QTime::fromString(time, "hh:mm");
    if (!qtime.isValid())
    {
        QMessageBox::warning(this, "Time Error", "Please enter time in HH:MM format.");
        return;
    }

    // Check if slot already exists
    for (const auto &slot : all_slots_)
    {
        if (slot->getDate() == date && slot->getTime() == time)
        {
            QMessageBox::warning(this, "Duplicate Slot", "This time slot already exists.");
            return;
        }
    }

    // Create new slot
    auto new_slot = std::make_shared<TimeSlot>(next_slot_id_++, time, date);
    all_slots_.push_back(new_slot);
    slotsByDate_[date].push(new_slot);

    // Clear input fields
    time_input_->clear();
    date_input_->setText(QDate::currentDate().toString("yyyy-MM-dd"));

    status_label_->setText(QString("Added slot: %1 %2").arg(date, time));
    refreshDisplay();
}

void CovidTestScheduler::bookSlot()
{
    QString patient_name = patient_name_input_->text().trimmed();
    int patient_age = patient_age_input_->value();
    QString selected_date = date_select_edit_->date().toString("yyyy-MM-dd");

    if (patient_name.isEmpty())
    {
        QMessageBox::warning(this, "Input Error", "Please enter patient name.");
        return;
    }

    if (slotsByDate_.count(selected_date) == 0 || slotsByDate_[selected_date].empty())
    {
        QMessageBox::information(this, "No Slots Available",
                                 "Sorry, no time slots are currently available for the selected date.");
        return;
    }

    // Get the selected slot from the combo box
    int selected_index = available_slots_combo_->currentIndex();
    if (selected_index < 0)
    {
        QMessageBox::warning(this, "Selection Error", "Please select a time slot.");
        return;
    }
    // Find the slot ID from the combo box data
    QVariant slot_id_var = available_slots_combo_->itemData(selected_index);
    if (!slot_id_var.isValid())
    {
        QMessageBox::warning(this, "Selection Error", "Invalid slot selection.");
        return;
    }
    int selected_slot_id = slot_id_var.toInt();

    // Find the slot in the heap for the selected date
    std::vector<std::shared_ptr<TimeSlot>> temp_slots;
    std::shared_ptr<TimeSlot> selected_slot = nullptr;
    while (!slotsByDate_[selected_date].empty())
    {
        auto slot = slotsByDate_[selected_date].top();
        slotsByDate_[selected_date].pop();
        if (slot->getId() == selected_slot_id && !slot->isBooked())
        {
            selected_slot = slot;
            // Do not push back, this is the one to book
        }
        else
        {
            temp_slots.push_back(slot);
        }
    }
    // Push the rest back into the heap
    for (auto &slot : temp_slots)
    {
        slotsByDate_[selected_date].push(slot);
    }

    if (!selected_slot)
    {
        QMessageBox::warning(this, "Slot Error", "The selected slot is no longer available.");
        refreshDisplay();
        return;
    }

    // Mark slot as booked
    selected_slot->setBooked(true);

    // Create patient booking
    auto patient = std::make_shared<Patient>(patient_name, patient_age, selected_slot);
    patient_bookings_.push_back(patient);

    // Clear input fields
    patient_name_input_->clear();
    patient_age_input_->setValue(25);

    status_label_->setText(QString("Booked slot for %1 on %2 at %3")
                               .arg(patient_name)
                               .arg(selected_slot->getDate())
                               .arg(selected_slot->getTime()));

    QMessageBox::information(this, "Booking Confirmed",
                             QString("Appointment booked for %1\n"
                                     "Date: %2\n"
                                     "Time: %3\n"
                                     "Slot ID: %4")
                                 .arg(patient_name)
                                 .arg(selected_slot->getDate())
                                 .arg(selected_slot->getTime())
                                 .arg(selected_slot->getId()));

    refreshDisplay();
}

void CovidTestScheduler::viewBookings()
{
    if (patient_bookings_.empty())
    {
        QMessageBox::information(this, "No Bookings", "No patient bookings found.");
        return;
    }

    updateBookingsTable();
    bookings_group_->setFocus();
}

void CovidTestScheduler::cancelSlot()
{
    if (patient_bookings_.empty())
    {
        QMessageBox::information(this, "No Bookings", "No bookings to cancel.");
        return;
    }

    // Get list of booked slots for selection
    QStringList booking_list;
    for (size_t i = 0; i < patient_bookings_.size(); ++i)
    {
        auto patient = patient_bookings_[i];
        auto slot = patient->getAssignedSlot();
        if (slot)
        {
            booking_list << QString("%1 - %2 (%3 %4)")
                                .arg(patient->getName())
                                .arg(patient->getAge())
                                .arg(slot->getDate())
                                .arg(slot->getTime());
        }
    }

    bool ok;
    QString selected = QInputDialog::getItem(this, "Cancel Booking",
                                             "Select booking to cancel:",
                                             booking_list, 0, false, &ok);

    if (ok && !selected.isEmpty())
    {
        int index = booking_list.indexOf(selected);
        if (index >= 0 && index < static_cast<int>(patient_bookings_.size()))
        {
            auto patient = patient_bookings_[index];

            // Find and unbook the slot
            auto slot = patient->getAssignedSlot();
            if (slot)
            {
                slot->setBooked(false);
                slotsByDate_[slot->getDate()].push(slot);

                status_label_->setText(QString("Cancelled booking for %1").arg(patient->getName()));

                // Remove patient from bookings
                patient_bookings_.erase(patient_bookings_.begin() + index);

                QMessageBox::information(this, "Booking Cancelled",
                                         QString("Booking cancelled for %1").arg(patient->getName()));

                refreshDisplay();
            }
        }
    }
}

void CovidTestScheduler::refreshDisplay()
{
    updateAvailableSlotsForSelectedDate();
    updateBookingsTable();
}

void CovidTestScheduler::updateAvailableSlotsForSelectedDate()
{
    available_slots_list_->clear();
    available_slots_combo_->clear();
    QString selected_date = date_select_edit_->date().toString("yyyy-MM-dd");
    int available_count = 0;
    if (slotsByDate_.count(selected_date) && !slotsByDate_[selected_date].empty())
    {
        // Create a copy of the priority queue to display all available slots for the selected date
        auto temp_queue = slotsByDate_[selected_date];
        int position = 1;
        while (!temp_queue.empty())
        {
            auto slot = temp_queue.top();
            temp_queue.pop();
            QString item_text = QString("%1. %2 %3 (ID: %4)")
                                    .arg(position++)
                                    .arg(slot->getDate())
                                    .arg(slot->getTime())
                                    .arg(slot->getId());
            available_slots_list_->addItem(item_text);
            available_slots_combo_->addItem(
                QString("%1 %2 (ID: %3)").arg(slot->getDate()).arg(slot->getTime()).arg(slot->getId()),
                slot->getId());
            ++available_count;
        }
    }
    else
    {
        available_slots_list_->addItem("No available slots");
        available_slots_combo_->addItem("No slots available");
    }
    available_slots_count_label_->setText(QString("Available Slots: %1").arg(available_count));
}

void CovidTestScheduler::updateBookingsTable()
{
    // Update table to show: Patient Name, Age, Slot Date, Slot Time
    bookings_table_->setColumnCount(4);
    QStringList headers;
    headers << "Patient Name" << "Age" << "Slot Date" << "Slot Time";
    bookings_table_->setHorizontalHeaderLabels(headers);
    bookings_table_->setRowCount(static_cast<int>(patient_bookings_.size()));

    for (size_t i = 0; i < patient_bookings_.size(); ++i)
    {
        auto patient = patient_bookings_[i];
        auto slot = patient->getAssignedSlot();
        bookings_table_->setItem(i, 0, new QTableWidgetItem(patient->getName()));
        bookings_table_->setItem(i, 1, new QTableWidgetItem(QString::number(patient->getAge())));
        bookings_table_->setItem(i, 2, new QTableWidgetItem(slot ? slot->getDate() : ""));
        bookings_table_->setItem(i, 3, new QTableWidgetItem(slot ? slot->getTime() : ""));
    }
}

void CovidTestScheduler::updateDateTime()
{
    QString current_datetime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    datetime_label_->setText(current_datetime);
}

#include "covid_test_scheduler.moc"
