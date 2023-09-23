// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <KConfig>
#include <KConfigGroup>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <physis.hpp>

#include "mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    KConfig config("novusrc");
    KConfigGroup game = config.group("Game");

    if (!game.hasKey("GameDir")) {
        while (true) {
            QMessageBox msgBox;
            msgBox.setText("The game directory has not been set, please select it now. Select the 'game' folder.");
            msgBox.exec();

            const QString dir = QFileDialog::getExistingDirectory(
                nullptr,
                "Open Game Directory",
                QStandardPaths::standardLocations(QStandardPaths::StandardLocation::HomeLocation).last(),
                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

            const std::string dirStd = dir.toStdString();

            if (!physis_gamedata_initialize(dirStd.c_str()))
                continue;

            game.writeEntry("GameDir", dir);
            config.sync();

            break;
        }
    }

    MainWindow window;
    window.show();

    return app.exec();
}
