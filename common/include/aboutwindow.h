// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDialog>

class AboutWindow : public QDialog {
public:
    explicit AboutWindow(QWidget* widget = nullptr);
};