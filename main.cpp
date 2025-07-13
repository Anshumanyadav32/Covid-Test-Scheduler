#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QFont>
#include <QIcon>
#include <QPixmap>
#include <QPalette>
#include <QDebug>
#include "covid_test_scheduler.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application properties
    app.setApplicationName("Covid Test Center Scheduler");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Covid Test Center");
    app.setOrganizationDomain("covidtestcenter.com");

    // Set application style
    app.setStyle(QStyleFactory::create("Fusion"));

    // Set application font
    QFont font("Segoe UI", 9);
    app.setFont(font);

    // Set application palette for better appearance
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    // Comment out the next line if you prefer light theme
    // app.setPalette(darkPalette);

    // Create and show the main window
    CovidTestScheduler window;
    window.show();

    qDebug() << "Covid Test Center Scheduler started successfully";
    qDebug() << "Qt Version:" << QT_VERSION_STR;
    qDebug() << "Application Name:" << app.applicationName();
    qDebug() << "Application Version:" << app.applicationVersion();

    return app.exec();
}
