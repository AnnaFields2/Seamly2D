/***************************************************************************
 **  @file   mapplication.cpp
 **  @author Douglas S Caskey
 **  @date   17 Sep, 2023
 **
 **  @copyright
 **  Copyright (C) 2017 - 2023 Seamly, LLC
 **  https://github.com/fashionfreedom/seamly2d
 **
 **  @brief
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Seamly2D is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

/************************************************************************
 **
 **  @file   mapplication.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   8 7, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "mapplication.h"
#include "version.h"
#include "tmainwindow.h"
#include "../ifc/exception/vexceptionobjecterror.h"
#include "../ifc/exception/vexceptionbadid.h"
#include "../ifc/exception/vexceptionconversionerror.h"
#include "../ifc/exception/vexceptionemptyparameter.h"
#include "../ifc/exception/vexceptionwrongid.h"
#include "../vmisc/logging.h"
#include "../vmisc/vsysexits.h"
#include "../vmisc/diagnostic.h"
#include "../qmuparser/qmuparsererror.h"

#include <Qt>
#include <QDir>
#include <QFileOpenEvent>
#include <QLocalSocket>
#include <QResource>
#include <QTranslator>
#include <QPointer>
#include <QLocalServer>
#include <QMessageBox>
#include <iostream>
#include <QGridLayout>
#include <QSpacerItem>
#include <QThread>
#include <QStandardPaths>

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(mApp, "m.application")

QT_WARNING_POP

#include <QCommandLineParser>

//---------------------------------------------------------------------------------------------------------------------
inline void noisyFailureMsgHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context)

    // Why on earth didn't Qt want to make failed signal/slot connections qWarning?
    if ((type == QtDebugMsg) && msg.contains(QStringLiteral("::connect")))
    {
        type = QtWarningMsg;
    }

#if defined(V_NO_ASSERT)
    // I have decided to hide this annoing message for release builds.
    if ((type == QtWarningMsg) && msg.contains(QStringLiteral("QSslSocket: cannot resolve")))
    {
        type = QtDebugMsg;
    }

    if ((type == QtWarningMsg) && msg.contains(QStringLiteral("setGeometry: Unable to set geometry")))
    {
        type = QtDebugMsg;
    }
#endif //defined(V_NO_ASSERT)

#if defined(Q_OS_MAC)
    // Hide Qt bug 'Assertion when reading an icns file'
    // https://bugreports.qt.io/browse/QTBUG-45537
    // Remove after Qt fix will be released
    if ((type == QtWarningMsg) && msg.contains(QStringLiteral("QICNSHandler::read()")))
    {
        type = QtDebugMsg;
    }

    // See issue #568
    if (msg.contains(QStringLiteral("Error receiving trust for a CA certificate")))
    {
        type = QtDebugMsg;
    }
#endif

    // this is another one that doesn't make sense as just a debug message.  pretty serious
    // sign of a problem
    // http://www.developer.nokia.com/Community/Wiki/QPainter::begin:Paint_device_returned_engine_%3D%3D_0_(Known_Issue)
    if ((type == QtDebugMsg) && msg.contains(QStringLiteral("QPainter::begin"))
        && msg.contains(QStringLiteral("Paint device returned engine")))
    {
        type = QtWarningMsg;
    }

    // This qWarning about "Cowardly refusing to send clipboard message to hung application..."
    // is something that can easily happen if you are debugging and the application is paused.
    // As it is so common, not worth popping up a dialog.
    if ((type == QtWarningMsg) && msg.contains(QStringLiteral("QClipboard::event"))
            && msg.contains(QStringLiteral("Cowardly refusing")))
    {
        type = QtDebugMsg;
    }

    // only the GUI thread should display message boxes.  If you are
    // writing a multithreaded application and the error happens on
    // a non-GUI thread, you'll have to queue the message to the GUI
    QCoreApplication *instance = QCoreApplication::instance();
    const bool isGuiThread = instance && (QThread::currentThread() == instance->thread());

    switch (type)
    {
        case QtDebugMsg:
            vStdOut() << QApplication::translate("mNoisyHandler", "DEBUG:") << msg << "\n";
            return;
        case QtWarningMsg:
            vStdErr() << QApplication::translate("mNoisyHandler", "WARNING:") << msg << "\n";
            break;
        case QtCriticalMsg:
            vStdErr() << QApplication::translate("mNoisyHandler", "CRITICAL:") << msg << "\n";
            break;
        case QtFatalMsg:
            vStdErr() << QApplication::translate("mNoisyHandler", "FATAL:") << msg << "\n";
            break;
        #if QT_VERSION > QT_VERSION_CHECK(5, 4, 2)
        case QtInfoMsg:
            vStdOut() << QApplication::translate("mNoisyHandler", "INFO:") << msg << "\n";
            break;
        #endif
        default:
            break;
    }

    if (isGuiThread)
    {
        //fixme: trying to make sure there are no save/load dialogs are opened, because error message during them will
        //lead to crash
        const bool topWinAllowsPop = (QApplication::activeModalWidget() == nullptr) ||
                !QApplication::activeModalWidget()->inherits("QFileDialog");
        QMessageBox messageBox;

        switch (type)
        {
            case QtWarningMsg:
                messageBox.setWindowTitle(QApplication::translate("mNoisyHandler", "Warning"));
                messageBox.setIcon(QMessageBox::Warning);
                break;
            case QtCriticalMsg:
                messageBox.setWindowTitle(QApplication::translate("mNoisyHandler", "Critical Error"));
                messageBox.setIcon(QMessageBox::Critical);
                break;
            case QtFatalMsg:
                messageBox.setWindowTitle(QApplication::translate("mNoisyHandler", "Fatal Error"));
                messageBox.setIcon(QMessageBox::Critical);
                break;
            #if QT_VERSION > QT_VERSION_CHECK(5, 4, 2)
            case QtInfoMsg:
                messageBox.setWindowTitle(QApplication::translate("mNoisyHandler", "Information"));
                messageBox.setIcon(QMessageBox::Information);
                break;
            #endif
            case QtDebugMsg:
                Q_UNREACHABLE(); //-V501
                break;
            default:
                break;
        }

        if (type == QtWarningMsg || type == QtCriticalMsg || type == QtFatalMsg)
        {
            if (not qApp->IsTestMode())
            {
                if (topWinAllowsPop)
                {
                    messageBox.setText(msg);
                    messageBox.setStandardButtons(QMessageBox::Ok);
                    messageBox.setWindowModality(Qt::ApplicationModal);
                    messageBox.setModal(true);
                #ifndef QT_NO_CURSOR
                    QGuiApplication::setOverrideCursor(Qt::ArrowCursor);
                #endif
                    messageBox.setWindowFlags(messageBox.windowFlags() & ~Qt::WindowContextHelpButtonHint);
                    messageBox.exec();
                #ifndef QT_NO_CURSOR
                    QGuiApplication::restoreOverrideCursor();
                #endif
                }
            }
        }

        if (QtFatalMsg == type)
        {
            abort();
        }
    }
    else
    {
		if (type != QtDebugMsg && type != QtWarningMsg)
        {
            abort(); // be NOISY unless overridden!
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
MApplication::MApplication(int &argc, char **argv)
    : VAbstractApplication(argc, argv)
    , mainWindows()
    , localServer(nullptr)
    , trVars(nullptr)
    , dataBase(QPointer<MeasurementDatabaseDialog>())
    , testMode(false)
{
    //setApplicationDisplayName(VER_PRODUCTNAME_STR);
    setApplicationName(VER_INTERNALNAME_ME_STR);
    setOrganizationName(VER_COMPANYNAME_STR);
    setOrganizationDomain(VER_COMPANYDOMAIN_STR);
    // Setting the Application version
    setApplicationVersion(APP_VERSION_STR);
    // We have been running SeamlyMe in two different cases.
    // The first inside own bundle where info.plist is works fine, but the second,
    // when we run inside Seamly2D's bundle, require direct setting the icon.
    setWindowIcon(QIcon(":/seamlymeicon/64x64/logo.png"));
}

//---------------------------------------------------------------------------------------------------------------------
MApplication::~MApplication()
{
    for (int i = 0; i < mainWindows.size(); ++i)
    {
        TMainWindow *window = mainWindows.at(i);
        delete window;
    }

    delete trVars;
    if (not dataBase.isNull())
    {
        delete dataBase;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief notify Reimplemented from QApplication::notify().
 * @param receiver receiver.
 * @param event event.
 * @return value that is returned from the receiver's event handler.
 */
// reimplemented from QApplication so we can throw exceptions in slots
bool MApplication::notify(QObject *receiver, QEvent *event)
{
    try
    {
        return QApplication::notify(receiver, event);
    }
    catch (const VExceptionObjectError &error)
    {
        qCCritical(mApp, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error parsing file. Program will be terminated.")), //-V807
                   qUtf8Printable(error.ErrorMessage()), qUtf8Printable(error.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VExceptionBadId &error)
    {
        qCCritical(mApp, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error bad id. Program will be terminated.")),
                   qUtf8Printable(error.ErrorMessage()), qUtf8Printable(error.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VExceptionConversionError &error)
    {
        qCCritical(mApp, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error can't convert value. Program will be terminated.")),
                   qUtf8Printable(error.ErrorMessage()), qUtf8Printable(error.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VExceptionEmptyParameter &error)
    {
        qCCritical(mApp, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error empty parameter. Program will be terminated.")),
                   qUtf8Printable(error.ErrorMessage()), qUtf8Printable(error.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VExceptionWrongId &error)
    {
        qCCritical(mApp, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error wrong id. Program will be terminated.")),
                   qUtf8Printable(error.ErrorMessage()), qUtf8Printable(error.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VExceptionToolWasDeleted &error)
    {
        qCCritical(mApp, "%s\n\n%s\n\n%s",
                   qUtf8Printable("Unhadled deleting tool. Continue use object after deleting!"),
                   qUtf8Printable(error.ErrorMessage()), qUtf8Printable(error.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VException &error)
    {
        qCCritical(mApp, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Something's wrong!!")),
                   qUtf8Printable(error.ErrorMessage()), qUtf8Printable(error.DetailedInformation()));
        return true;
    }
    // These last two cases special. I found that we can't show here modal dialog with error message.
    // Somehow program doesn't waite untile an error dialog will be closed. But if ignore this program will hang.
    catch (const qmu::QmuParserError &error)
    {
        qCCritical(mApp, "%s", qUtf8Printable(tr("Parser error: %1. Program will be terminated.").arg(error.GetMsg())));
        exit(V_EX_DATAERR);
    }
    catch (std::exception &error)
    {
        qCCritical(mApp, "%s", qUtf8Printable(tr("Exception thrown: %1. Program will be terminated.").arg(error.what())));
        exit(V_EX_SOFTWARE);
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
bool MApplication::IsTestMode() const
{
    return testMode;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief IsAppInGUIMode little hack that allow to have access to application state from VAbstractApplication class.
 */
bool MApplication::IsAppInGUIMode() const
{
    return IsTestMode();
}

//---------------------------------------------------------------------------------------------------------------------
TMainWindow *MApplication::MainWindow()
{
    Clean();
    if (mainWindows.isEmpty())
    {
        NewMainWindow();
    }
    return mainWindows[0];
}

//---------------------------------------------------------------------------------------------------------------------
QList<TMainWindow *> MApplication::MainWindows()
{
    Clean();
    QList<TMainWindow*> list;
    for (int i = 0; i < mainWindows.count(); ++i)
    {
        list.append(mainWindows.at(i));
    }
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
void MApplication::InitOptions()
{
    qInstallMessageHandler(noisyFailureMsgHandler);

    OpenSettings();
    VSeamlyMeSettings *settings = SeamlyMeSettings();
    QDir().mkpath(settings->getDefaultTemplatePath());
    QDir().mkpath(settings->getDefaultIndividualSizePath());
    QDir().mkpath(settings->getDefaultMultisizePath());
    QDir().mkpath(settings->getDefaultLabelTemplatePath());

    qCInfo(mApp, "Version: %s", qUtf8Printable(APP_VERSION_STR));
    qCInfo(mApp, "Build revision: %s", BUILD_REVISION);
    qCInfo(mApp, "%s", qUtf8Printable(buildCompatibilityString()));
    qCInfo(mApp, "Built on %s at %s", __DATE__, __TIME__);
    qCInfo(mApp, "Command-line arguments: %s", qUtf8Printable(arguments().join(", ")));
    qCInfo(mApp, "Process ID: %s", qUtf8Printable(QString().setNum(applicationPid())));

    loadTranslations(QLocale().name());// By default the console version uses system locale

    static const char * GENERIC_ICON_TO_CHECK = "document-open";
    if (QIcon::hasThemeIcon(GENERIC_ICON_TO_CHECK) == false)
    {
       //If there is no default working icon theme then we should
       //use an icon theme that we provide via a .qrc file
       //This case happens under Windows and Mac OS X
       //This does not happen under GNOME or KDE
       QIcon::setThemeName("win.icon.theme");
    }
}

//---------------------------------------------------------------------------------------------------------------------
const VTranslateVars *MApplication::TrVars()
{
    return trVars;
}

//---------------------------------------------------------------------------------------------------------------------
void MApplication::InitTrVars()
{
    if (trVars != nullptr)
    {
        trVars->Retranslate();
    }
    else
    {
        trVars = new VTranslateVars();
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool MApplication::event(QEvent *event)
{
    switch(event->type())
    {
        // In Mac OS X the QFileOpenEvent event is generated when user perform "Open With" from Finder (this event is
        // Mac specific).
        case QEvent::FileOpen:
        {
            QFileOpenEvent *fileOpenEvent = static_cast<QFileOpenEvent *>(event);
            const QString macFileOpen = fileOpenEvent->file();
            if(not macFileOpen.isEmpty())
            {
                TMainWindow *mw = MainWindow();
                if (mw)
                {
                    mw->LoadFile(macFileOpen);  // open file in existing window
                }
                return true;
            }
            break;
        }
#if defined(Q_OS_MAC)
        case QEvent::ApplicationActivate:
        {
            Clean();
            TMainWindow *mw = MainWindow();
            if (mw && not mw->isMinimized())
            {
                mw->show();
            }
            return true;
        }
#endif //defined(Q_OS_MAC)
        default:
            return VAbstractApplication::event(event);
    }
    return VAbstractApplication::event(event);
}

//---------------------------------------------------------------------------------------------------------------------
void MApplication::OpenSettings()
{
    settings = new VSeamlyMeSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(),
                                 QCoreApplication::applicationName(), this);
}

//---------------------------------------------------------------------------------------------------------------------
VSeamlyMeSettings *MApplication::SeamlyMeSettings()
{
    SCASSERT(settings != nullptr)
    return qobject_cast<VSeamlyMeSettings *>(settings);
}

//---------------------------------------------------------------------------------------------------------------------
void MApplication::ShowDataBase()
{
    if (dataBase.isNull())
    {
        dataBase = new MeasurementDatabaseDialog();
        dataBase->setAttribute(Qt::WA_DeleteOnClose, true);
        dataBase->setModal(false);
        dataBase->show();
    }
    else
    {
        dataBase->activateWindow();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MApplication::retranslateGroups()
{
    if (not dataBase.isNull())
    {
        dataBase->retranslateGroups();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MApplication::RetranslateTables()
{
    QList<TMainWindow*> list = MainWindows();
    for (int i=0; i < list.size(); ++i)
    {
        list.at(i)->RetranslateTable();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MApplication::ParseCommandLine(const SocketConnection &connection, const QStringList &arguments)
{
    QCommandLineParser parser;
    parser.setApplicationDescription(tr("Seamly2D's measurements editor."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("filename", tr("The measurement file."));
    //-----
    QCommandLineOption heightOption(QStringList() << "e" << "height",
            tr("Open with the base height. Valid values: %1cm.")
                                    .arg(MeasurementVariable::WholeListHeights(Unit::Cm).join(", ")),
            tr("The base height"));
    parser.addOption(heightOption);
    //-----
    QCommandLineOption sizeOption(QStringList() << "s" << "size",
            tr("Open with the base size. Valid values: %1cm.").arg(MeasurementVariable::WholeListSizes(Unit::Cm).join(", ")),
            tr("The base size"));
    parser.addOption(sizeOption);
    //-----
    QCommandLineOption unitOption(QStringList() << "u" << "unit",
            tr("Set pattern file unit: cm, mm, inch."),
            tr("The pattern unit"));
    parser.addOption(unitOption);
    //-----
    QCommandLineOption testOption(QStringList() << "test",
            tr("Use for unit testing. Run the program and open a file without showing the main window."));
    parser.addOption(testOption);
    //-----
    QCommandLineOption scalingOption(QStringList() << LONG_OPTION_NO_HDPI_SCALING,
            tr("Disable high dpi scaling. Call this option if has problem with scaling (by default scaling enabled). "
               "Alternatively you can use the %1 environment variable.").arg("QT_AUTO_SCREEN_SCALE_FACTOR=0"));
    parser.addOption(scalingOption);
    //-----
    parser.process(arguments);

    bool flagHeight = false;
    bool flagSize = false;
    bool flagUnit = false;

    int size = 0;
    int height = 0;
    Unit unit = Unit::Cm;

    if (parser.isSet(heightOption))
    {
        const QString heightValue = parser.value(heightOption);
        if (MeasurementVariable::IsGradationHeightValid(heightValue))
        {
            flagHeight = true;
            height = heightValue.toInt();
        }
        else
        {
            qCCritical(mApp, "%s\n",
                    qPrintable(tr("Invalid base height argument. Must be %1cm.")
                               .arg(MeasurementVariable::WholeListHeights(Unit::Cm).join(", "))));
            parser.showHelp(V_EX_USAGE);
        }
    }

    if (parser.isSet(sizeOption))
    {
        const QString sizeValue = parser.value(sizeOption);
        if (MeasurementVariable::IsGradationSizeValid(sizeValue))
        {
            flagSize = true;
            size = sizeValue.toInt();
        }
        else
        {
            qCCritical(mApp, "%s\n",
                    qPrintable(tr("Invalid base size argument. Must be %1cm.")
                               .arg(MeasurementVariable::WholeListSizes(Unit::Cm).join(", "))));
            parser.showHelp(V_EX_USAGE);
        }
    }

    {
    const QString unitValue = parser.value(unitOption);
    if (not unitValue.isEmpty())
    {

        const QStringList units = QStringList() << unitMM << unitCM << unitINCH;
        if (units.contains(unitValue))
        {
            flagUnit = true;
            unit = StrToUnits(unitValue);
        }
        else
        {
            qCCritical(mApp, "%s\n", qPrintable(tr("Invalid base size argument. Must be cm, mm or inch.")));
            parser.showHelp(V_EX_USAGE);
        }
    }
    }

    testMode = parser.isSet(testOption);

    if (not testMode && connection == SocketConnection::Client)
    {
        const QString serverName = QCoreApplication::applicationName();
        QLocalSocket socket;
        socket.connectToServer(serverName);
        if (socket.waitForConnected(1000))
        {
            qCInfo(mApp, "Connected to the server '%s'", qUtf8Printable(serverName));
            QTextStream stream(&socket);
            stream << QCoreApplication::arguments().join(";;");
            stream.flush();
            socket.waitForBytesWritten();
            qApp->exit(V_EX_OK);
            return;
        }

        qCDebug(mApp, "Can't establish connection to the server '%s'", qUtf8Printable(serverName));

        localServer = new QLocalServer(this);
        connect(localServer, &QLocalServer::newConnection, this, &MApplication::NewLocalSocketConnection);
        if (not localServer->listen(serverName))
        {
            qCWarning(mApp, "Can't begin to listen for incoming connections on name '%s'",
                    qUtf8Printable(serverName));
            if (localServer->serverError() == QAbstractSocket::AddressInUseError)
            {
                QLocalServer::removeServer(serverName);
                if (not localServer->listen(serverName))
                {
                    qCWarning(mApp, "%s",
                     qUtf8Printable(tr("Can't begin to listen for incoming connections on name '%1'").arg(serverName)));
                }
            }
        }

        //loadTranslations(SeamlyMeSettings()->GetLocale());
    }

    const QStringList args = parser.positionalArguments();
    if (args.count() > 0)
    {
        if (testMode && args.count() > 1)
        {
            qCCritical(mApp, "%s\n", qPrintable(tr("Test mode doesn't support Opening several files.")));
            parser.showHelp(V_EX_USAGE);
        }

        for (int i = 0; i < args.size(); ++i)
        {
            NewMainWindow();
            if (not MainWindow()->LoadFile(args.at(i)))
            {
                if (testMode)
                {
                    return; // process only one input file
                }
                delete MainWindow();
                continue;
            }

            if (flagSize)
            {
                MainWindow()->SetBaseMSize(size);
            }

            if (flagHeight)
            {
                MainWindow()->SetBaseMHeight(height);
            }

            if (flagUnit)
            {
                MainWindow()->SetPUnit(unit);
            }
        }
    }
    else
    {
        if (not testMode)
        {
            NewMainWindow();
        }
        else
        {
            qCCritical(mApp, "%s\n", qPrintable(tr("Please, provide one input file.")));
            parser.showHelp(V_EX_USAGE);
        }
    }

    if (testMode)
    {
        qApp->exit(V_EX_OK); // close program after processing in console mode
    }
}

//---------------------------------------------------------------------------------------------------------------------
TMainWindow *MApplication::NewMainWindow()
{
    TMainWindow *seamlyme = new TMainWindow();
    mainWindows.prepend(seamlyme);
    if (not qApp->IsTestMode())
    {
        seamlyme->show();
    }
    return seamlyme;
}

//---------------------------------------------------------------------------------------------------------------------
void MApplication::processCommandLine()
{
    ParseCommandLine(SocketConnection::Client, arguments());
}

//---------------------------------------------------------------------------------------------------------------------
void MApplication::NewLocalSocketConnection()
{
    QLocalSocket *socket = localServer->nextPendingConnection();
    if (not socket)
    {
        return;
    }
    socket->waitForReadyRead(1000);
    QTextStream stream(socket);
    const QString arg = stream.readAll();
    if (not arg.isEmpty())
    {
        ParseCommandLine(SocketConnection::Server, arg.split(";;"));
    }
    delete socket;
    MainWindow()->raise();
    MainWindow()->activateWindow();
}

//---------------------------------------------------------------------------------------------------------------------
void MApplication::Clean()
{
    // cleanup any deleted main windows first
    for (int i = mainWindows.count() - 1; i >= 0; --i)
    {
        if (mainWindows.at(i).isNull())
        {
            mainWindows.removeAt(i);
        }
    }
}
