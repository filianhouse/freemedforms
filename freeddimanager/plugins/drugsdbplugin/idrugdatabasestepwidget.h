/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2013 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
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
 *   Main developers : Eric Maeker
 *  Contributors:                                                          *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 ***************************************************************************/
#ifndef DDIMANAGER_DRUGSDB_INTERNAL_IDRUGDATABASESTEPWIDGET_H
#define DDIMANAGER_DRUGSDB_INTERNAL_IDRUGDATABASESTEPWIDGET_H

#include <QWidget>

/**
 * \file idrugdatabasestepwidget.h
 * \author Eric Maeker
 * \version 0.10.0
 * \date 10 Jan 2014
*/

namespace DrugsDb {
namespace Internal {
class IDrugDatabaseStep;
class IDrugDatabaseStepWidgetPrivate;

class IDrugDatabaseStepWidget : public QWidget
{
    Q_OBJECT
public:
    explicit IDrugDatabaseStepWidget(QWidget *parent = 0);
    ~IDrugDatabaseStepWidget();

    bool initialize(IDrugDatabaseStep *step);

private Q_SLOTS:
    void on_startJobs_clicked();
    bool on_download_clicked();
    void downloadFinished();
    void changeStepProgressRange(int min, int max);

private:
    void showEvent(QShowEvent *event);

private:
    Internal::IDrugDatabaseStepWidgetPrivate *d;
};

} // namespace Internal
} // namespace DrugsDb

#endif // DDIMANAGER_DRUGSDB_INTERNAL_IDRUGDATABASESTEPWIDGET_H

