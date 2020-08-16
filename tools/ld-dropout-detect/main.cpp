/************************************************************************

    main.cpp

    ld-dropout-detect - LaserDisc dropout detection tools
    Copyright (C) 2020 Simon Inns

    This file is part of ld-decode-tools.

    ld-dropout-detect is free software: you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

************************************************************************/

#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QtGlobal>
#include <QCommandLineParser>

#include "logging.h"

int main(int argc, char *argv[])
{
    // Install the local debug message handler
    setDebug(true);
    qInstallMessageHandler(debugOutputHandler);

    QApplication a(argc, argv);

    // Set application name and version
    QCoreApplication::setApplicationName("ld-dropout-correct");
    QCoreApplication::setApplicationVersion(QString("Branch: %1 / Commit: %2").arg(APP_BRANCH, APP_COMMIT));
    QCoreApplication::setOrganizationDomain("domesday86.com");

    // Set up the command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription(
                "ld-dropout-detect - LaserDisc dropout detection tools\n"
                "\n"
                "(c)2020 Simon Inns\n"
                "GPLv3 Open-Source - github: https://github.com/happycube/ld-decode");
    parser.addHelpOption();
    parser.addVersionOption();

    // Add the standard debug options --debug and --quiet
    addStandardDebugOptions(parser);

    // Option to run in interactive mode (-g / --interactive)
    QCommandLineOption interactiveOption(QStringList() << "g" << "interactive",
                                       QCoreApplication::translate("main", "Run in interactive mode"));
    parser.addOption(interactiveOption);

    // Option to reverse the field order (-r / --reverse)
    QCommandLineOption setReverseOption(QStringList() << "r" << "reverse",
                                       QCoreApplication::translate("main", "Reverse the field order to second/first (default first/second)"));
    parser.addOption(setReverseOption);

    // -- Positional arguments --

    // Positional argument to specify the input EFM filea
    parser.addPositionalArgument("input", QCoreApplication::translate("main", "Specify 1 or more input TBC files (maximum 64)"));

    // Process the command line options and arguments given by the user
    parser.process(a);

    // Standard logging options
    processStandardDebugOptions(parser);

    // Get the options from the parser
    bool interactiveMode = parser.isSet(interactiveOption);
    bool reverse = parser.isSet(setReverseOption);

    QVector<QString> inputFilenames;
    QStringList positionalArguments = parser.positionalArguments();

    // Get the arguments from the parser
    if (positionalArguments.count() > 64) {
        qCritical() << "A maximum of 64 input sources are supported";
        return 1;
    }

    if (positionalArguments.count() >= 1) {
        for (qint32 i = 0; i < positionalArguments.count(); i++) {
            inputFilenames.append(positionalArguments.at(i));
        }
    } else {
        // Quit with error
        qCritical("You must specify at least 1 input TBC file");
        return 1;
    }

    // Initialise the GUI application
    MainWindow w;

    // Configure the GUI application
    w.configure(interactiveMode, reverse, inputFilenames);

    // Show the main GUI window
    if (interactiveMode) w.show();

    // Process the TBC source files
    if (!w.process()) return 1;

    if (interactiveMode) return a.exec();
    return 0;
}
