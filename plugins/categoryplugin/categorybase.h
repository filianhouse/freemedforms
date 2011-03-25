/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2011 by Eric MAEKER, MD (France) <eric.maeker@free.fr>        *
 *  All rights reserved.                                                   *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program (COPYING.FREEMEDFORMS file).                   *
 *  If not, see <http://www.gnu.org/licenses/>.                            *
 ***************************************************************************/
/***************************************************************************
 *   Main Developper : Eric MAEKER, <eric.maeker@free.fr>                  *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/
#ifndef CATEGORY_BASE_H
#define CATEGORY_BASE_H

#include <utils/database.h>
#include <QVector>
#include <QList>


namespace Category {
class CategoryItem;
namespace Internal {
class CategoryBasePrivate;

class CategoryBase : public QObject, public Utils::Database
{
    Q_OBJECT

protected:
    CategoryBase(QObject *parent = 0);

public:
    // Constructor
    static CategoryBase *instance();
    virtual ~CategoryBase();

    // initialize
    bool init();


private:
    bool createDatabase(const QString &connectionName, const QString &dbName,
                          const QString &pathOrHostName,
                          TypeOfAccess access, AvailableDrivers driver,
                          const QString &login, const QString &pass,
                          const int port,
                          CreationOption createOption
                         );

public:
    QVector<CategoryItem *> getCategories(const QString &mime) const;
    QList<CategoryItem *> createCategoryTree(const QVector<CategoryItem *> &cats) const;
    bool saveCategory(CategoryItem *category);
    bool updateCategory(CategoryItem *category);
    bool saveCategoryLabels(CategoryItem *category);

private Q_SLOTS:
    void onCoreDatabaseServerChanged();

private:
    static bool m_initialized;
    static CategoryBase *m_Instance;
};

}  // End namespace Internal
}  // End namespace Category


#endif // CATEGORY_BASE_H
