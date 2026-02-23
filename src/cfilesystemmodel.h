/***************************************************************************
 *   Copyright (C) 2015 by Joerg Zopes                                     *
 *   joerg.zopes@gmx.de                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#pragma once

#include <QFileSystemModel>
#include <QModelIndex>
#include <QSet>
#include <QPersistentModelIndex>

class CFileSystemModel : public QFileSystemModel
{
public:
    CFileSystemModel() = default;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    void setChecked(const QModelIndex &index, bool state);
    void setChecked(QString path, bool state);
    void unCheckAll();
    QSet<QPersistentModelIndex> checkedIndexes() const { return m_checkedIndexes; }

private:

    QSet<QPersistentModelIndex> m_checkedIndexes;

    bool recursiveCheck(const QModelIndex &index, const QVariant &value);
};
