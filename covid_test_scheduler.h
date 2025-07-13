#ifndef COVID_TEST_SCHEDULER_H
#define COVID_TEST_SCHEDULER_H

#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QHeaderView>
#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <queue>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <QtWidgets/QDateEdit>

// Forward declarations
class TimeSlot;
class Patient;
class CovidTestScheduler;

/**
 * @brief TimeSlot class represents a Covid test appointment slot
 */
class TimeSlot
{
public:
    TimeSlot(int id, const QString &time, const QString &date);

    int getId() const { return id_; }
    QString getTime() const { return time_; }
    QString getDate() const { return date_; }
    QString getDateTime() const { return date_ + " " + time_; }
    bool isBooked() const { return is_booked_; }

    void setBooked(bool booked) { is_booked_ = booked; }

    // Comparison operators for min-heap (earlier time has higher priority)
    bool operator>(const TimeSlot &other) const;
    bool operator<(const TimeSlot &other) const;
    bool operator==(const TimeSlot &other) const;

private:
    int id_;
    QString time_;
    QString date_;
    bool is_booked_;
};

/**
 * @brief Patient class represents a patient booking
 */
class Patient
{
public:
    Patient(const QString &name, int age, std::shared_ptr<TimeSlot> assignedSlot);

    QString getName() const { return name_; }
    int getAge() const { return age_; }
    std::shared_ptr<TimeSlot> getAssignedSlot() const { return assigned_slot_; }
    QString getBookingTime() const { return booking_time_; }

private:
    QString name_;
    int age_;
    std::shared_ptr<TimeSlot> assigned_slot_;
    QString booking_time_;
};

/**
 * @brief Custom comparator for min-heap of TimeSlot objects
 */
class TimeSlotComparator
{
public:
    bool operator()(const std::shared_ptr<TimeSlot> &a, const std::shared_ptr<TimeSlot> &b)
    {
        return *a > *b; // Min-heap: smaller element has higher priority
    }
};

/**
 * @brief Main application class for Covid Test Center Scheduler
 */
class CovidTestScheduler : public QMainWindow
{
    Q_OBJECT

public:
    CovidTestScheduler(QWidget *parent = nullptr);
    ~CovidTestScheduler();
    void addSampleSlots();

private slots:
    void addSlot();
    void bookSlot();
    void viewBookings();
    void cancelSlot();
    void refreshDisplay();
    void updateDateTime();

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void updateAvailableSlots();
    void updateBookingsTable();
    void showAvailableSlots();
    void updateAvailableSlotsForSelectedDate(); // NEW: update available slots for selected date

    // UI Components
    QWidget *central_widget_;
    QVBoxLayout *main_layout_;
    QHBoxLayout *button_layout_;
    QGridLayout *content_layout_;

    // Control buttons
    QPushButton *add_slot_button_;
    QPushButton *book_slot_button_;
    QPushButton *view_bookings_button_;
    QPushButton *cancel_slot_button_;
    QPushButton *refresh_button_;

    // Input fields
    QGroupBox *add_slot_group_;
    QLineEdit *time_input_;
    QLineEdit *date_input_;
    QDateEdit *date_select_edit_; // NEW: for user to select date

    QGroupBox *book_patient_group_;
    QLineEdit *patient_name_input_;
    QSpinBox *patient_age_input_;
    QComboBox *available_slots_combo_;

    // Display areas
    QGroupBox *slots_group_;
    QListWidget *available_slots_list_;

    QGroupBox *bookings_group_;
    QTableWidget *bookings_table_;

    // Status and info
    QLabel *status_label_;
    QLabel *datetime_label_;
    QLabel *available_slots_count_label_; // NEW: show number of available slots
    QTimer *datetime_timer_;

    // Data structures
    // Remove single heap, use map of heaps by date
    std::map<QString, std::priority_queue<std::shared_ptr<TimeSlot>, std::vector<std::shared_ptr<TimeSlot>>, TimeSlotComparator>> slotsByDate_;
    std::vector<std::shared_ptr<Patient>> patient_bookings_;
    std::vector<std::shared_ptr<TimeSlot>> all_slots_;

    int next_slot_id_;
};

#endif // COVID_TEST_SCHEDULER_H
