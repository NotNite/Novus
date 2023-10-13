// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDoubleSpinBox>
#include <QWidget>
#include <physis.hpp>

class RaceTreeData : public QObject
{
    Q_OBJECT

public:
    RaceTreeData(Race race, Subrace subrace)
        : race(race)
        , subrace(subrace)
    {
    }

    Race race;
    Subrace subrace;
};

class CmpEditor : public QWidget
{
    Q_OBJECT

public:
    explicit CmpEditor(GameData *data, QWidget *parent = nullptr);

private:
    void loadRaceData(Race race, Subrace subrace);

    GameData *data = nullptr;
    physis_CMP cmp;

    QDoubleSpinBox *maleMinSize = nullptr;
    QDoubleSpinBox *maleMaxSize = nullptr;

    QDoubleSpinBox *maleMinTail = nullptr;
    QDoubleSpinBox *maleMaxTail = nullptr;

    QDoubleSpinBox *femaleMinSize = nullptr;
    QDoubleSpinBox *femaleMaxSize = nullptr;

    QDoubleSpinBox *femaleMinTail = nullptr;
    QDoubleSpinBox *femaleMaxTail = nullptr;

    QDoubleSpinBox *bustMinX = nullptr;
    QDoubleSpinBox *bustMinY = nullptr;
    QDoubleSpinBox *bustMinZ = nullptr;

    QDoubleSpinBox *bustMaxX = nullptr;
    QDoubleSpinBox *bustMaxY = nullptr;
    QDoubleSpinBox *bustMaxZ = nullptr;
};
