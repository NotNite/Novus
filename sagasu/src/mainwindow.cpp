// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <QApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>

#include "cmppart.h"
#include "exdpart.h"
#include "exlpart.h"
#include "filepropertieswindow.h"
#include "filetreewindow.h"
#include "hexpart.h"
#include "mdlpart.h"
#include "shpkpart.h"
#include "sklbpart.h"
#include "texpart.h"

MainWindow::MainWindow(const QString &gamePath, GameData *data)
    : NovusMainWindow()
    , data(data)
    , fileCache(*data)
{
    setupMenubar();

    auto dummyWidget = new QWidget();
    setCentralWidget(dummyWidget);

    auto layout = new QHBoxLayout();
    dummyWidget->setLayout(layout);

    auto tree = new FileTreeWindow(m_database, gamePath, data);
    connect(tree, &FileTreeWindow::extractFile, this, [this, data](const QString &path) {
        const QFileInfo info(path);

        const QString savePath = QFileDialog::getSaveFileName(this, tr("Save File"), info.fileName(), QStringLiteral("*.%1").arg(info.completeSuffix()));
        if (!savePath.isEmpty()) {
            qInfo() << "Saving to" << savePath;

            std::string savePathStd = path.toStdString();

            auto fileData = physis_gamedata_extract_file(data, savePathStd.c_str());
            QFile file(savePath);
            file.open(QIODevice::WriteOnly);
            file.write(reinterpret_cast<const char *>(fileData.data), fileData.size);
        }
    });
    connect(tree, &FileTreeWindow::pathSelected, this, [this](const QString &path) {
        refreshParts(path);
    });
    tree->setMaximumWidth(200);
    layout->addWidget(tree);

    partHolder = new QTabWidget();
    partHolder->setMinimumWidth(800);
    partHolder->setMinimumHeight(720);
    layout->addWidget(partHolder);

    refreshParts({});
}

void MainWindow::refreshParts(const QString &path)
{
    partHolder->clear();

    std::string pathStd = path.toStdString();
    if (path.isEmpty() || !physis_gamedata_exists(data, pathStd.c_str())) {
        return;
    }

    auto file = physis_gamedata_extract_file(data, path.toStdString().c_str());

    QFileInfo info(path);
    if (info.completeSuffix() == QStringLiteral("exl")) {
        auto exlWidget = new EXLPart(data);
        exlWidget->load(file);
        partHolder->addTab(exlWidget, QStringLiteral("Excel List"));
    } else if (info.completeSuffix() == QStringLiteral("exh")) {
        auto exdWidget = new EXDPart(data);
        exdWidget->loadSheet(info.baseName(), file);
        partHolder->addTab(exdWidget, QStringLiteral("Excel Sheet"));
    } else if (info.completeSuffix() == QStringLiteral("exd")) {
        auto exdWidget = new QLabel(QStringLiteral("Note: Excel data files cannot be previewed standalone, select the EXH file instead."));
        partHolder->addTab(exdWidget, QStringLiteral("Note"));
    } else if (info.completeSuffix() == QStringLiteral("mdl")) {
        auto mdlWidget = new MDLPart(data, fileCache);
        mdlWidget->addModel(physis_mdl_parse(file), false, glm::vec3(), QStringLiteral("mdl"), {}, 0);
        partHolder->addTab(mdlWidget, QStringLiteral("Model"));
    } else if (info.completeSuffix() == QStringLiteral("tex") || info.completeSuffix() == QStringLiteral("atex")) {
        auto texWidget = new TexPart(data);
        texWidget->load(file);
        partHolder->addTab(texWidget, QStringLiteral("Texture"));
    } else if (info.completeSuffix() == QStringLiteral("shpk")) {
        auto shpkWidget = new SHPKPart(data);
        shpkWidget->load(file);
        partHolder->addTab(shpkWidget, QStringLiteral("Shader Package"));
    } else if (info.completeSuffix() == QStringLiteral("cmp")) {
        auto cmpWidget = new CmpPart(data);
        cmpWidget->load(file);
        partHolder->addTab(cmpWidget, QStringLiteral("Chara Make Params"));
    } else if (info.completeSuffix() == QStringLiteral("sklb")) {
        auto sklbWidget = new SklbPart();
        sklbWidget->load(physis_parse_skeleton(file));
        partHolder->addTab(sklbWidget, QStringLiteral("Skeleton"));
    }

    auto hexWidget = new HexPart();
    hexWidget->loadFile(file);
    partHolder->addTab(hexWidget, QStringLiteral("Raw Hex"));

    auto propertiesWidget = new FilePropertiesWindow(path, file);
    partHolder->addTab(propertiesWidget, QStringLiteral("Properties"));
}

void MainWindow::setupFileMenu(QMenu *menu)
{
    auto openList = menu->addAction(QStringLiteral("Import Path List..."));
    openList->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
    connect(openList, &QAction::triggered, [this] {
        auto fileName = QFileDialog::getOpenFileName(nullptr, QStringLiteral("Open Path List"), QStringLiteral("~"));

        QMessageBox::warning(this,
                             QStringLiteral("Import Warning"),
                             QStringLiteral("Depending on the size of the import, this process usually takes a few minutes. The program may freeze. Please "
                                            "keep it open until the operation is finished."),
                             QMessageBox::Ok,
                             QMessageBox::Ok);

        m_database.importFileList(fileName);

        QMessageBox::information(this, QStringLiteral("Import Complete"), QStringLiteral("Successfully imported path list!"), QMessageBox::Ok, QMessageBox::Ok);
    });

    auto downloadList = menu->addAction(QStringLiteral("Download Path List..."));
    downloadList->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
    connect(downloadList, &QAction::triggered, [this] {
        auto fileName = QFileDialog::getOpenFileName(nullptr, QStringLiteral("Open Path List"), QStringLiteral("~"));

        m_database.importFileList(fileName);
    });
}
