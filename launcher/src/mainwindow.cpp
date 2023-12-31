// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <KConfig>
#include <KConfigGroup>
#include <QComboBox>
#include <QDebug>
#include <QDesktopServices>
#include <QFormLayout>
#include <QListWidget>
#include <QProcess>
#include <QUrl>
#include <QVBoxLayout>

static QMap<QString, QPair<QString, QString>> applications = {
    {QStringLiteral("Gear Editor"), {QStringLiteral("zone.xiv.armoury"), QStringLiteral("novus-armoury")}},
    {QStringLiteral("Excel Editor"), {QStringLiteral("zone.xiv.karaku"), QStringLiteral("novus-karuku")}},
    {QStringLiteral("Data Explorer"), {QStringLiteral("zone.xiv.sagasu"), QStringLiteral("novus-sagasu")}},
    {QStringLiteral("Model Viewer"), {QStringLiteral("zone.xiv.mdlviewer"), QStringLiteral("novus-mdlviewer")}}};

static QMap<QString, QString> links = {{QStringLiteral("XIV Dev Wiki"), QStringLiteral("https://xiv.dev")},
                                       {QStringLiteral("XIV Docs"), QStringLiteral("https://docs.xiv.zone")}};

MainWindow::MainWindow()
    : NovusMainWindow()
{
    setupMenubar();

    auto appList = new QListWidget();

    auto applicationHeader = new QListWidgetItem();
    applicationHeader->setText(QStringLiteral("Applications"));
    applicationHeader->setFlags(Qt::NoItemFlags);

    appList->addItem(applicationHeader);

    for (const auto &key : applications.keys()) {
        auto application = new QListWidgetItem();
        application->setText(key);
        application->setIcon(QIcon::fromTheme(applications[key].first));

        appList->addItem(application);
    }

    auto linksHeader = new QListWidgetItem();
    linksHeader->setText(QStringLiteral("Links"));
    linksHeader->setFlags(Qt::NoItemFlags);

    appList->addItem(linksHeader);

    for (const auto &key : links.keys()) {
        auto application = new QListWidgetItem();
        application->setText(key);
        application->setIcon(QIcon::fromTheme(QStringLiteral("internet-web-browser")));

        appList->addItem(application);
    }

    connect(appList, &QListWidget::itemClicked, [this](QListWidgetItem *item) {
        if (applications.contains(item->text())) {
            const QString exec = QStringLiteral("./") + applications[item->text()].second;

            qDebug() << "Launching" << exec;

            QProcess::startDetached(exec, QStringList());
        } else if (links.contains(item->text())) {
            QDesktopServices::openUrl(QUrl::fromUserInput(links[item->text()]));
        }
    });

    auto appListLayout = new QVBoxLayout();
    appListLayout->addWidget(appList);

    auto centralFrame = new QFrame();
    centralFrame->setLayout(appListLayout);

    auto formLayout = new QFormLayout();

    KConfig config(QStringLiteral("novusrc"));
    KConfigGroup game = config.group(QStringLiteral("Game"));

    auto gameCombo = new QComboBox();
    gameCombo->setMaximumWidth(175);
    formLayout->addRow(QStringLiteral("Current Game"), gameCombo);
    formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    gameCombo->addItem(game.readEntry("GameDir"));

    auto mainLayout = new QVBoxLayout();
    mainLayout->addWidget(centralFrame);
    mainLayout->addLayout(formLayout);
    auto centralWidget = new QWidget();
    centralWidget->setLayout(mainLayout);

    setCentralWidget(centralWidget);
}

#include "moc_mainwindow.cpp"