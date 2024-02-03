// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <KZip>
#include <QApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QUrl>
#include <physis.hpp>

#include "exdpart.h"
#include "sheetlistwidget.h"

MainWindow::MainWindow(GameData *data)
    : NovusMainWindow()
    , data(data)
{
    setMinimumSize(1280, 720);
    setupMenubar();

    mgr = new QNetworkAccessManager(this);

    auto dummyWidget = new QWidget();
    setCentralWidget(dummyWidget);

    auto layout = new QHBoxLayout();
    dummyWidget->setLayout(layout);

    auto listWidget = new SheetListWidget(data);
    listWidget->setMaximumWidth(200);
    layout->addWidget(listWidget);

    auto exdPart = new EXDPart(data);
    layout->addWidget(exdPart);

    connect(listWidget, &SheetListWidget::sheetSelected, this, [data, exdPart](const QString &name) {
        auto path = QStringLiteral("exd/%1.exh").arg(name.toLower());
        auto pathStd = path.toStdString();

        auto file = physis_gamedata_extract_file(data, pathStd.c_str());

        exdPart->loadSheet(name, file);
    });
}

void MainWindow::setupFileMenu(QMenu *menu)
{
    auto openList = menu->addAction(QStringLiteral("Import Definitions..."));
    openList->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
    connect(openList, &QAction::triggered, [this] {
        auto fileName = QFileDialog::getOpenFileName(nullptr, QStringLiteral("Open Path List"), QStringLiteral("~"));
    });

    auto downloadList = menu->addAction(QStringLiteral("Download Definitions..."));
    downloadList->setIcon(QIcon::fromTheme(QStringLiteral("download-symbolic")));
    connect(downloadList, &QAction::triggered, [this] {
        const int ret =
            QMessageBox::information(this,
                                     QStringLiteral("Download Confirmation"),
                                     QStringLiteral("This will download the definitions from the  <a "
                                                    "href=\"https://github.com/xivapi/SaintCoinach\">SaintCoinach repository on GitHub</a>.<br><br>Continue?"),
                                     QMessageBox::Ok | QMessageBox::Cancel,
                                     QMessageBox::Ok);

        if (ret != QMessageBox::Ok) {
            return;
        }

        QUrl url;
        url.setScheme(QStringLiteral("https"));
        url.setHost(QStringLiteral("github.com"));
        url.setPath(QStringLiteral("/xivapi/SaintCoinach/releases/latest/download/Godbert.zip"));

        // TODO: Use Qcoro?
        auto reply = mgr->get(QNetworkRequest(url));
        connect(reply, &QNetworkReply::finished, this, [this, reply] {
            qInfo() << "Finished downloading definitions!";

            QTemporaryDir tempDir;

            QFile file(tempDir.filePath(QStringLiteral("Godbert.zip")));
            file.open(QIODevice::WriteOnly);
            file.write(reply->readAll());
            file.close();

            KZip archive(tempDir.filePath(QStringLiteral("Godbert.zip")));
            if (!archive.open(QIODevice::ReadOnly)) {
                // TODO: these should show as message boxes
                qFatal() << "Failed to open Godbert zip!";
                return;
            }

            const KArchiveDirectory *root = dynamic_cast<const KArchiveDirectory *>(archive.directory()->entry(QStringLiteral("Definitions")));

            const QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            const QDir definitionsDir = dataDir.absoluteFilePath(QStringLiteral("definitions"));

            // delete old directory
            if (definitionsDir.exists()) {
                QDir().rmdir(definitionsDir.absolutePath());
            }

            QDir().mkpath(definitionsDir.absolutePath());

            root->copyTo(definitionsDir.absolutePath(), true);

            archive.close();

            QMessageBox::information(this, QStringLiteral("Definitions"), QStringLiteral("Successfully updated definitions!"));
        });
    });
}

#include "moc_mainwindow.cpp"